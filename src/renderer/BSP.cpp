#include "BSP.h"

#include <cstdlib>
#include <utility>

#include <SDL3/SDL.h>

#include "common/Debug.h"

namespace BSP
{
	namespace
	{
		/*
		* Создает коробку вокруг уровня с заданным отступом
		*/
		FaceList createWorldBounds(const Bounds::AABB3& src, float padding, bool flip)
		{
			FaceList polys;
			Bounds::AABB3 aabb = src;

			aabb.min -= padding;
			aabb.max += padding;

			Vec3 v000 = Vec3(aabb.min.x, aabb.min.y, aabb.min.z);
			Vec3 v001 = Vec3(aabb.min.x, aabb.min.y, aabb.max.z);
			Vec3 v010 = Vec3(aabb.min.x, aabb.max.y, aabb.min.z);
			Vec3 v011 = Vec3(aabb.min.x, aabb.max.y, aabb.max.z);

			Vec3 v100 = Vec3(aabb.max.x, aabb.min.y, aabb.min.z);
			Vec3 v101 = Vec3(aabb.max.x, aabb.min.y, aabb.max.z);
			Vec3 v110 = Vec3(aabb.max.x, aabb.max.y, aabb.min.z);
			Vec3 v111 = Vec3(aabb.max.x, aabb.max.y, aabb.max.z);

			auto addPoly = [&](const Vec3& vec1, const Vec3& vec2, const Vec3& vec3, const Vec3& vec4) {
				if (flip) {
					polys.push_back(Face(Poly3D({ {vec4, {0, 0}}, {vec1, {0, 1}}, {vec2, {1, 1}}, {vec3, {1, 0}} }), 1, FaceType::Wall));
				}
				else {
					polys.push_back(Face(Poly3D({ {vec4, {0, 0}}, {vec3, {0, 1}}, {vec2, {1, 1}}, {vec1, {1, 0}} }), 1, FaceType::Wall));
				}	
			};

			addPoly(v000, v001, v011, v010);
			addPoly(v100, v110, v111, v101);
			addPoly(v000, v100, v101, v001);
			addPoly(v010, v011, v111, v110);
			addPoly(v000, v010, v110, v100);
			addPoly(v001, v101, v111, v011);

			return polys;
		}

		/*
		* Плоскости world bounds
		*/
		PlaneList getPlanes(const Bounds::AABB3& aabb)
		{
			PlaneList planes;

			planes.push_back(Plane(Vec3(-1, 0, 0), -aabb.max.x));
			planes.push_back(Plane(Vec3(1, 0, 0), aabb.min.x));

			planes.push_back(Plane(Vec3(0, -1, 0), -aabb.max.y));
			planes.push_back(Plane(Vec3(0, 1, 0), aabb.min.y));

			planes.push_back(Plane(Vec3(0, 0, -1), -aabb.max.z));
			planes.push_back(Plane(Vec3(0, 0, 1), aabb.min.z));

			return planes;
		}

		/*
		* Рекурсивно делим пространство и заполняем дерево
		*/
		void buildRecursive(const FaceList& faces, const Context& ctx)
		{
			if (faces.size() == 0) {
				return;
			}

			if (ctx.depth > maxDepth) {
				Debug::print("BSP depth limit exceeded");
				return;
			}

			size_t nodeIdx = ctx.nodeIdx;

			Splitter splitter = findSplitter(faces);

			// Сплиттер найден - делим пространство
			if (splitter.valid) {

				// Создаем пустые дочерние ноды и сразу присваем родительской
				int frontNodeIdx = ctx.tree.addNode();
				int backNodeIdx = ctx.tree.addNode();

				ctx.tree.getNode(nodeIdx).frontIdx = frontNodeIdx;
				ctx.tree.getNode(nodeIdx).backIdx = backNodeIdx;

				FaceList frontFaces, backFaces, coplanarFaces;
				Context frontCtx(ctx.tree), backCtx(ctx.tree);

				// Родительская нода - не лист
				ctx.tree.getNode(nodeIdx).leafIdx = -1;
				ctx.tree.getNode(nodeIdx).plane = splitter.plane;

				// Разбиваем полигоны сплиттером и распределяем их по дочерним спискам.
				for (const Face& face : faces) {

					SplittedPoly3D splittedPoly = Geometry::split(face.poly, splitter.plane);

					// Особый случай: полигон лежит на плоскости сплиттера. Если он ориентирован в ту же сторону, добавляем в передний список, иноче - в задний.
					if (splittedPoly.result == SplitResult3D::Coplanar) {
						Face coplanarFace = Face(face, splittedPoly.coplanar);

						// Копланарный полигон больше не может делить пространство.
						coplanarFace.canSplit = false;

						coplanarFaces.push_back(coplanarFace);

						if (Vec3::dot(coplanarFace.poly.getPlane().normal, splitter.plane.normal) < 0) {
							backFaces.push_back(coplanarFace);
						}
						else {
							frontFaces.push_back(coplanarFace);
						}

					}
					else if (splittedPoly.result == SplitResult3D::Front) {
						frontFaces.push_back(Face(face, splittedPoly.front));
					}
					else if (splittedPoly.result == SplitResult3D::Back) {
						backFaces.push_back(Face(face, splittedPoly.back));
					}
					else if (splittedPoly.result == SplitResult3D::Intersected) {
						frontFaces.push_back(Face(face, splittedPoly.front));
						backFaces.push_back(Face(face, splittedPoly.back));
					}
				}

				ctx.tree.getNode(nodeIdx).bounds = ctx.bounds;
				ctx.tree.getNode(nodeIdx).coplanarFaces = coplanarFaces;

				// Формируем bounds для дочерних элементов
				frontCtx.depth = ctx.depth + 1;
				frontCtx.nodeIdx = frontNodeIdx;
				frontCtx.bounds = ctx.bounds;
				frontCtx.bounds.push_back(splitter.plane);

				backCtx.depth = ctx.depth + 1;
				backCtx.nodeIdx = backNodeIdx;
				backCtx.bounds = ctx.bounds;
				backCtx.bounds.push_back(splitter.plane.flip());

				buildRecursive(frontFaces, frontCtx);
				buildRecursive(backFaces, backCtx);

				// Присваем AABB, объеденяющий AABB дочерних нод
				ctx.tree.getNode(nodeIdx).aabb = Bounds::create({ ctx.tree.getNode(frontNodeIdx).aabb, ctx.tree.getNode(backNodeIdx).aabb });
			}
			// Нечего разбивать - создаем лист
			else {
				int leafIdx = ctx.tree.addLeaf();

				ctx.tree.getNode(nodeIdx).frontIdx = -1;
				ctx.tree.getNode(nodeIdx).backIdx = -1;

				// Делаем целый лист outside если в его составе есть соответствующий полигон
				for (const auto& face : faces) {
				
					// Добавляем в лист текущие bounds и создаем его геометрию
					ctx.tree.getLeaf(leafIdx).bounds.push_back(face.poly.getPlane());
					ctx.tree.getLeaf(leafIdx).faceIdxs.push_back(ctx.tree.addFace(face, leafIdx));
				}

				ctx.tree.getNode(nodeIdx).leafIdx = leafIdx;

				// Присваиваем AABB листа
				ctx.tree.getNode(nodeIdx).aabb = Bounds::create(faces);
			}
		}

		void buildPortalsFromNodeRecursive(Tree& tree, int nodeIdx)
		{
			if (tree.getNode(nodeIdx).leafIdx != -1) {
				return;
			}

			Portal portal = createPortal(tree, nodeIdx);

			pushPortal(tree, portal, tree.getNode(nodeIdx).frontIdx, tree.getNode(nodeIdx).backIdx);

			buildPortalsFromNodeRecursive(tree, tree.getNode(nodeIdx).frontIdx);
			buildPortalsFromNodeRecursive(tree, tree.getNode(nodeIdx).backIdx);
		}

		void pushPortal(Tree& tree, Portal portal, int frontNodeIdx, int backNodeIdx)
		{
			bool frontNodeIsLeaf = (tree.getNode(frontNodeIdx).leafIdx != -1);
			bool backNodeIsLeaf = (tree.getNode(backNodeIdx).leafIdx != -1);

			if (frontNodeIsLeaf && backNodeIsLeaf) {
				int frontLeafIdx = tree.getNode(frontNodeIdx).leafIdx;
				int backLeafIdx = tree.getNode(backNodeIdx).leafIdx;

				if (frontLeafIdx != backLeafIdx) {
					portal.frontNodeIdx = frontNodeIdx;
					portal.backNodeIdx = backNodeIdx;
					portal.frontLeafIdx = frontLeafIdx;
					portal.backLeafIdx = backLeafIdx;

					if (clipPortal(tree, portal)) {
						PortalList finalPortals = carvePortal(tree, portal);

						for (const auto& finalPortal : finalPortals) {
							int portalIdx = tree.addPortal(finalPortal);
							int flippedPortalIdx = tree.addPortal(flipPortal(finalPortal));

							tree.getPortal(portalIdx).flippedIdx = flippedPortalIdx;
							tree.getPortal(flippedPortalIdx).flippedIdx = portalIdx;

							tree.getLeaf(frontLeafIdx).portalIdxs.push_back(flippedPortalIdx);
							tree.getLeaf(backLeafIdx).portalIdxs.push_back(portalIdx);
						}
					}
				}

				return;
			}

			if (!frontNodeIsLeaf) {
				refinePortalSide(tree, portal, frontNodeIdx, backNodeIdx, PortalSide::Front);
				return;
			}

			if (!backNodeIsLeaf) {
				refinePortalSide(tree, portal, backNodeIdx, frontNodeIdx, PortalSide::Back);
				return;
			}
		}

		/*
		void pushFace(const BSPFace& face, int nodeIdx)
		{
			if (getNode(nodeIdx).leafIdx == -1) {
				SplitResult3D classification = Geometry::classify(Poly3D::offset(face.face.poly, 0.01f), getNode(nodeIdx).plane);



				if (classification == SplitResult3D::Coplanar) {

					if (Vec3::dot(face.face.poly.getPlane().normal, getNode(nodeIdx).plane.normal) > 0) {
						pushFace(face, getNode(nodeIdx).frontIdx);
					}
					else {
						pushFace(face, getNode(nodeIdx).backIdx);
					}
				}
				else if (classification == SplitResult3D::Front) {
					pushFace(face, getNode(nodeIdx).frontIdx);
				}
				else if (classification == SplitResult3D::Back) {
					pushFace(face, getNode(nodeIdx).backIdx);
				}
				else if (classification == SplitResult3D::Intersected) {
					SplittedPoly3D splittedPoly = Geometry::split(face.face.poly, getNode(nodeIdx).plane);
					pushFace(BSPFace(Face(face.face, splittedPoly.front), getNode(nodeIdx).frontIdx), getNode(nodeIdx).frontIdx);
					pushFace(BSPFace(Face(face.face, splittedPoly.back), getNode(nodeIdx).backIdx), getNode(nodeIdx).backIdx);
				}
			}
			else {
				if (getLeaf(getNode(nodeIdx).leafIdx).valid) {
					getLeaf(getNode(nodeIdx).leafIdx).faceIdxs.push_back(addBSPFace(face.face, getNode(nodeIdx).leafIdx));
				}
			}

		}
		*/

		/*
		* Создание портала
		* Берем огромный полигон на плоскости ноды (сплиттера) и обрезаем всеми родительскими плоскостями
		*/
		Portal createPortal(const Tree& tree, int nodeIdx)
		{
			Portal portal;

			const Node& node = tree.getNode(nodeIdx);

			portal.nodeIdx = nodeIdx;
			portal.poly = Geometry::makePolygonOnPlane(node.plane, 100000.0f);
			portal.poly = Geometry::clip(portal.poly, node.bounds);

			return portal;
		}

		void refinePortalSide(Tree& tree, const Portal& portal, int splitNodeIdx, int otherNodeIdx, PortalSide side)
		{
			SplitResult3D cls = Geometry::classify(portal.poly, tree.getNode(splitNodeIdx).plane);

			if (cls == SplitResult3D::Front) {

				if (side == PortalSide::Front) {
					pushPortal(tree, portal, tree.getNode(splitNodeIdx).frontIdx, otherNodeIdx);
				}
				else {
					pushPortal(tree, portal, otherNodeIdx, tree.getNode(splitNodeIdx).frontIdx);
				}

				return;
			}

			if (cls == SplitResult3D::Back) {

				if (side == PortalSide::Front) {
					pushPortal(tree, portal, tree.getNode(splitNodeIdx).backIdx, otherNodeIdx);
				}
				else {
					pushPortal(tree, portal, otherNodeIdx, tree.getNode(splitNodeIdx).backIdx);
				}

				return;
			}

			if (cls == SplitResult3D::Intersected) {
				SplittedPoly3D splittedPortalPoly = Geometry::split(portal.poly, tree.getNode(splitNodeIdx).plane);
				Portal frontPortal, backPortal;

				frontPortal = backPortal = portal;

				frontPortal.poly = splittedPortalPoly.front;
				backPortal.poly = splittedPortalPoly.back;

				if (side == PortalSide::Front) {
					pushPortal(tree, frontPortal, tree.getNode(splitNodeIdx).frontIdx, otherNodeIdx);
					pushPortal(tree, backPortal, tree.getNode(splitNodeIdx).backIdx, otherNodeIdx);
				}
				else {
					pushPortal(tree, frontPortal, otherNodeIdx, tree.getNode(splitNodeIdx).frontIdx);
					pushPortal(tree, backPortal, otherNodeIdx, tree.getNode(splitNodeIdx).backIdx);
				}

				return;
			}

			// Невозможно
			if (cls == SplitResult3D::Coplanar) {
				pushPortal(tree, portal, tree.getNode(splitNodeIdx).frontIdx, otherNodeIdx);
				pushPortal(tree, portal, tree.getNode(splitNodeIdx).backIdx, otherNodeIdx);

				return;
			}
		}

		/*
		* Обрезание портала плоскостями граничащих с ним листьев
		*/
		bool clipPortal(const Tree& tree, Portal& portal)
		{
			const Leaf& frontLeaf = tree.getLeaf(portal.frontLeafIdx);
			const Leaf& backLeaf = tree.getLeaf(portal.backLeafIdx);

			PlaneList bounds;

			bounds.insert(bounds.end(), frontLeaf.bounds.begin(), frontLeaf.bounds.end());
			bounds.insert(bounds.end(), backLeaf.bounds.begin(), backLeaf.bounds.end());

			portal.poly = Geometry::clip(portal.poly, bounds);

			return portal.poly.isValid();
		}

		Portal flipPortal(const Portal& portal)
		{
			Portal flippedPortal = portal;

			std::swap(flippedPortal.frontNodeIdx, flippedPortal.backNodeIdx);
			std::swap(flippedPortal.frontLeafIdx, flippedPortal.backLeafIdx);

			std::reverse(flippedPortal.poly.verts.begin(), flippedPortal.poly.verts.end());

			return flippedPortal;
		}

		void pushFace(Tree& tree, const BSPFace& face, int nodeIdx)
		{
			if (tree.getNode(nodeIdx).leafIdx == -1) {
				SplitResult3D classification = Geometry::classify(Poly3D::offset(face.face.poly, 0.01f), tree.getNode(nodeIdx).plane);



				if (classification == SplitResult3D::Coplanar) {

					if (Vec3::dot(face.face.poly.getPlane().normal, tree.getNode(nodeIdx).plane.normal) > 0) {
						pushFace(tree, face, tree.getNode(nodeIdx).frontIdx);
					}
					else {
						pushFace(tree, face, tree.getNode(nodeIdx).backIdx);
					}
				}
				else if (classification == SplitResult3D::Front) {
					pushFace(tree, face, tree.getNode(nodeIdx).frontIdx);
				}
				else if (classification == SplitResult3D::Back) {
					pushFace(tree, face, tree.getNode(nodeIdx).backIdx);
				}
				else if (classification == SplitResult3D::Intersected) {
					SplittedPoly3D splittedPoly = Geometry::split(face.face.poly, tree.getNode(nodeIdx).plane);
					pushFace(tree, BSPFace(Face(face.face, splittedPoly.front), tree.getNode(nodeIdx).frontIdx), tree.getNode(nodeIdx).frontIdx);
					pushFace(tree, BSPFace(Face(face.face, splittedPoly.back), tree.getNode(nodeIdx).backIdx), tree.getNode(nodeIdx).backIdx);
				}
			}
			else {
				if (tree.getLeaf(tree.getNode(nodeIdx).leafIdx).contents == Contents::Empty) {
					tree.getLeaf(tree.getNode(nodeIdx).leafIdx).faceIdxs.push_back(tree.addFace(face.face, tree.getNode(nodeIdx).leafIdx));
				}
			}

		}

		void restoreBorderFaces(Tree& tree)
		{
			for (int leafIdx = 0; leafIdx < tree.getLeafCount(); leafIdx++) {
				const Leaf& leaf = tree.getLeaf(leafIdx);

				if (leaf.contents == Contents::Solid) {
					continue;
				}

				for (int faceIdx : leaf.faceIdxs) {
					Vec3 n = tree.faces[faceIdx].face.poly.getPlane().normal;
					Vec3 p = tree.faces[faceIdx].face.poly.getCenter();

					p += n * 0.1f;
					int otherLeafIdx = getLeafIdx(tree, p);

					if (otherLeafIdx == leafIdx) {
						continue;
					}

					const Leaf& otherLeaf = tree.getLeaf(otherLeafIdx);

					if (otherLeaf.contents == Contents::Solid) {
						continue;
					}

					auto face = tree.faces[faceIdx];
					face.face.poly = Poly3D::offset(face.face.poly, 0.01f);

					pushFace(tree, face, tree.rootNodeIdx);

					Debug::printf("Face returned from {} to {}", leafIdx, otherLeafIdx);
				}
			}
		}

		/*
		* Вырезание из портала solid-геометрии
		* Переводим портал и копланарную геометрию ноды в 2D-вид и последовательно вычитаем из портала каждый копланарный полигон
		* Результат - множество порталов которые уцелели
		*/
		PortalList carvePortal(const Tree& tree, const Portal& in)
		{
			PortalList out;

			Poly3DList pieces;
			pieces.push_back(in.poly);

			for (const auto& face : tree.getNode(in.nodeIdx).coplanarFaces)
			{
				if (face.type == FaceType::Liquid || face.type == FaceType::Entity) {
					continue;
				}

				Poly3DList next;

				for (const auto& piece : pieces) {
					Poly3DList polys = Geometry::subtract(piece, face.poly, in.poly.getPlane(), 0.1f);
					next.insert(next.end(), polys.begin(), polys.end());
				}

				pieces = std::move(next);

				if (pieces.empty()) {
					break;
				}
			}

			for (const auto& poly : pieces) {
				Portal p = in;
				p.poly = poly;

				out.push_back(p);
			}

			return out;
		}

		bool floodFill(Tree& tree, int startLeafIdx)
		{
			bool leakDetected = false;
			IndicesList stack;

			IndicesList visitedLeaves;
			visitedLeaves.push_back(startLeafIdx);

			for (int i = 0; i < tree.getLeafCount(); i++) {
				tree.getLeaf(i).contents = Contents::Solid;
			}

			stack.push_back(startLeafIdx);
			tree.getLeaf(startLeafIdx).contents = Contents::Empty;

			while (!stack.empty()) {
				int leafIdx = stack.back();
				stack.pop_back();

				for (int portalIdx : tree.getLeaf(leafIdx).portalIdxs) {
					const Portal& p = tree.getPortal(portalIdx);

					

					int nextLeafIdx = p.frontLeafIdx == leafIdx ? p.backLeafIdx : (p.backLeafIdx == leafIdx ? p.frontLeafIdx : -1);


					if (nextLeafIdx == -1) {
						continue;
					}

					if (std::find(visitedLeaves.begin(), visitedLeaves.end(), nextLeafIdx) != visitedLeaves.end()) {
						continue;
					}

					tree.getLeaf(nextLeafIdx).contents = Contents::Empty;
					visitedLeaves.push_back(nextLeafIdx);
					/*
					if (tree.getLeaf(nextLeafIdx).outside) {
						leakDetected = true;
					}
					*/

					stack.push_back(nextLeafIdx);
				}

			}

			return leakDetected;
		}

		void traverse(const Tree& tree, int nodeIndex, const Camera& cam, FaceList& renderFaces, const BoolList& pvsTableRow, bool usePvs)
		{
			if (nodeIndex == -1) {
				return;
			}

			const Node& node = tree.getNode(nodeIndex);

			if (!cam.aabbInFrustrum(node.aabb)) {
				return;
			}

			if (node.leafIdx != -1)
			{
				const Leaf& leaf = tree.getLeaf(node.leafIdx);

				if (!usePvs || pvsTableRow[node.leafIdx]) {
					for (int faceIdx : leaf.faceIdxs) {
						BSPFace bspFace = tree.faces[faceIdx];

						bspFace.face.debugColor = Color::getDebugColor(bspFace.leafIdx);

						renderFaces.push_back(bspFace.face);
					}
				}
			}
			else
			{
				Side side = Geometry::test(cam.position, node.plane);

				if (side == Side::Front) {
					traverse(tree, node.backIdx, cam, renderFaces, pvsTableRow, usePvs);
					traverse(tree, node.frontIdx, cam, renderFaces, pvsTableRow, usePvs);
				}
				else if (side == Side::Back) {
					traverse(tree, node.frontIdx, cam, renderFaces, pvsTableRow, usePvs);
					traverse(tree, node.backIdx, cam, renderFaces, pvsTableRow, usePvs);
				}
				else {
					traverse(tree, node.frontIdx, cam, renderFaces, pvsTableRow, usePvs);
					traverse(tree, node.backIdx, cam, renderFaces, pvsTableRow, usePvs);
				}
			}
		}

		Splitter findSplitter(const FaceList& faces)
		{
			Splitter splitter = {
				.valid = false,
				.idx = -1
			};

			if (faces.size() <= 1) {
				return splitter;
			}

			int minScore = faces.size() * 20; // FIX

			for (int j = 0; j < faces.size(); j++) {
				const Face& splitterFace = faces[j];
				Plane splitterPlane = splitterFace.poly.getPlane();

				if (!splitterFace.canSplit) {
					continue;
				}

				int coplanarCount = 0, frontCount = 0, backCount = 0, splitCount = 0;

				for (int i = 0; i < faces.size(); i++) {
					if (i == j) {
						continue;
					}


					SplittedPoly3D splitted = Geometry::split(faces[i].poly, splitterPlane);

					if (splitted.result == SplitResult3D::Coplanar) {
						coplanarCount++;
					}
					else if (splitted.result == SplitResult3D::Front) {
						frontCount++;
					}
					else if (splitted.result == SplitResult3D::Back) {
						backCount++;
					}
					else if (splitted.result == SplitResult3D::Intersected) {
						frontCount++;
						backCount++;
						splitCount++;
					}
				}

				if (backCount > 0) {
					int score = splitCount * 16 + abs(backCount - frontCount) - coplanarCount * 2;
					if (splitterFace.poly.getPlane().isAxial()) {
						score -= 4;
					}
					if (score < minScore) {
						splitter.valid = true;
						splitter.idx = j;
						splitter.plane = splitterPlane;
						minScore = score;
					}
				}
			}

			return splitter;
		}
	} // Private

	bool compile(Tree& tree, FaceList& faces, const Vec3List& playerStart)
	{
		tree.clear();

		FaceList worldBounds = createWorldBounds(Bounds::create(faces), 10.0f);
		faces.insert(faces.end(), worldBounds.begin(), worldBounds.end());

		Bounds::AABB3 worldAABB = Bounds::create(faces);

		Context ctx(tree);
		ctx.bounds = getPlanes(worldAABB);

		tree.rootNodeIdx = tree.addNode();
		ctx.nodeIdx = tree.rootNodeIdx;

		tree.portals.clear();

		Debug::print("Building BSP tree...");
		buildRecursive(faces, ctx);
		//restoreBorderFaces(tree);

		Debug::print("Building portals...");
		buildPortalsFromNodeRecursive(tree, tree.rootNodeIdx);

		bool leakFound = false;

		for (const auto& pos : playerStart) {
			leakFound = floodFill(tree, getLeafIdx(tree, pos)) || leakFound;
		}

		if (leakFound) {
			Debug::print("Leak detected!");
			return leakFound;
		}

		Debug::printf("Build completed.");
		Debug::printf("\tNodes: {}", tree.getNodeCount());
		Debug::printf("\tLeaves: {}", tree.getLeafCount());
		Debug::printf("\tFaces: {}", tree.getFaceCount());
		Debug::printf("\tPortals: {}", tree.getPortalCount());

		FaceList finalFaces;

		for (const auto& leaf : tree.getLeaves()) {

			if (leaf.contents == Contents::Solid) {
				continue;
			}

			for (auto faceIdx : leaf.faceIdxs) {
				Face finalFace = tree.faces[faceIdx].face;
				finalFace.canSplit = true;
				finalFaces.push_back(finalFace);
			}
		}

		tree.clear();

		worldAABB = Bounds::create(finalFaces);

		ctx.bounds = getPlanes(worldAABB);
		
		

		//tree.getNode(outsideNodeIdx).frontIdx = -1;
		//tree.getNode(outsideNodeIdx).backIdx = -1;
		//tree.getNode(outsideNodeIdx).leafIdx = outsideLeafIdx;

		size_t outsideLeafIdx = tree.addLeaf();
		tree.getLeaf(outsideLeafIdx).contents = Contents::Solid;

		tree.rootNodeIdx = tree.addNode();
		ctx.nodeIdx = tree.rootNodeIdx;
		ctx.depth = 0;

		Debug::print("Building final BSP tree...");
		buildRecursive(finalFaces, ctx);

		Debug::print("Building final portals...");
		buildPortalsFromNodeRecursive(tree, tree.rootNodeIdx);

		/*
		for (const auto& pos : playerStart) {
			leakFound = floodFill(tree, getLeafIdx(tree, pos)) || leakFound;
		}
		*/

		Debug::printf("Build completed.");
		Debug::printf("\tNodes: {}", tree.getNodeCount());
		Debug::printf("\tLeaves: {}", tree.getLeafCount());
		Debug::printf("\tFaces: {}", tree.getFaceCount());
		Debug::printf("\tPortals: {}", tree.getPortalCount());

		return leakFound;
	}


	void render(const Tree& tree, const Camera& cam, FaceList& renderFaces, const BoolList& pvsTableRow)
	{
		renderFaces.clear();

		bool usePvs = !(getLeafIdx(tree, cam.position) == 0);

		traverse(tree, tree.rootNodeIdx, cam, renderFaces, pvsTableRow, usePvs);
	}

	PortalList getPortalsInLeaf(const Tree& tree, int leafIdx)
	{
		PortalList portals;

		const Leaf& leaf = tree.getLeaf(leafIdx);

		for (int portalIdx : leaf.portalIdxs) {
			portals.push_back(tree.getPortal(portalIdx));
		}

		return portals;
	}

	size_t getLeafIdx(const Tree& tree, const Vec3& pos)
	{
		int nodeIdx = tree.rootNodeIdx;

		while (true) {
			const Node& node = tree.getNode(nodeIdx);

			if (node.leafIdx != -1) {

				if (!pointInLeaf(tree, node.leafIdx, pos)) {
					return 0;
				}

				return node.leafIdx;
			}

			if (Geometry::test(pos, node.plane) == Side::Back) {
				nodeIdx = node.backIdx;
			}
			else {
				nodeIdx = node.frontIdx;
			}
		}

		return 0;
	}

	bool pointInLeaf(const Tree& tree, size_t leafIdx, const Vec3& pos)
	{
		const Leaf& leaf = tree.getLeaf(leafIdx);

		for (auto faceIdx : leaf.faceIdxs) {

			if (Geometry::test(pos, tree.getFace(faceIdx).face.poly.getPlane()) == Side::Back) {
				return false;
			}
		}

		return true;
	}

	void findTouchedLeaves(const Tree& tree, size_t nodeIdx, const Bounds::AABB3& aabb, IndicesList& leafIdxs)
	{
		const Node& node = tree.getNode(nodeIdx);

		if (node.leafIdx != -1) {

			leafIdxs.push_back(node.leafIdx);
			return;
		}

		Vec3 center = aabb.getCenter();
		Vec3 extents = aabb.getExtents();

		const Plane& plane = node.plane;

		float dist = Math::distanceToPlane(plane, center);

		float radius = std::abs(plane.normal.x) * extents.x + std::abs(plane.normal.y) * extents.y + std::abs(plane.normal.z) * extents.z;

		if (dist > radius) {
			findTouchedLeaves(tree, node.frontIdx, aabb, leafIdxs);
		}
		else if (dist < -radius) {
			findTouchedLeaves(tree, node.backIdx, aabb, leafIdxs);
		}
		else {
			findTouchedLeaves(tree, node.frontIdx, aabb, leafIdxs);
			findTouchedLeaves(tree, node.backIdx, aabb, leafIdxs);
		}
	}
}