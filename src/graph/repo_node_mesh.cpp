/**
 *  Copyright (C) 2014 3D Repo Ltd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "repo_node_mesh.h"

#include <algorithm>
#include <functional>

//------------------------------------------------------------------------------
//
// Constructors
//
//------------------------------------------------------------------------------
repo::core::RepoNodeMesh::RepoNodeMesh(
	const unsigned int api,
	const aiMesh *mesh,
	const std::vector<RepoNodeAbstract *> & materials) :
		RepoNodeAbstract (
			REPO_NODE_TYPE_MESH,
			api,
            boost::uuids::random_generator()(),
			mesh->mName.data),
			vertices(NULL),
			faces(NULL),
			normals(NULL),
			outline(NULL),
            uvChannels(NULL),
            colors(NULL)
{
    //--------------------------------------------------------------------------
	// Vertices (always present)
	// TODO: make sure enough memory can be allocated
	vertices = new std::vector<aiVector3t<float>>();
	vertices->reserve(mesh->mNumVertices);
	vertices->insert(vertices->begin(),
		mesh->mVertices, mesh->mVertices + mesh->mNumVertices);


    //--------------------------------------------------------------------------
	// Faces
	if (mesh->HasFaces())
	{
		faces = new std::vector<aiFace>();
		faces->reserve(mesh->mNumFaces);
		faces->insert(faces->begin(),
			mesh->mFaces, mesh->mFaces + mesh->mNumFaces);
	}

    //--------------------------------------------------------------------------
	// Normals
	if (mesh->HasNormals())
	{
		normals = new std::vector<aiVector3t<float>>();
		normals->reserve(mesh->mNumVertices);
		normals->insert(normals->begin(),
			mesh->mNormals, mesh->mNormals + mesh->mNumVertices);
	}

    //--------------------------------------------------------------------------
	// Bones
	/*
	if (mesh->HasBones())
	{
	}


	if (mesh->HasPositions())
	{

	}

	if (mesh->HasTangentsAndBitangents())
	{

	}*/

    //--------------------------------------------------------------------------
	// UV channels
	// Copies only the very first UV channel over
	// TODO: make sure enough memory can be allocated
	// TODO: add support for all UV channels.
	if (mesh->HasTextureCoords(0))
	{
		uvChannels = new std::vector<std::vector<aiVector3t<float>> * >();
		uvChannels->push_back(new std::vector<aiVector3t<float>>());
		uvChannels->at(0)->reserve(mesh->mNumVertices);

		uvChannels->at(0)->insert(uvChannels->at(0)->begin(),
		mesh->mTextureCoords[0], mesh->mTextureCoords[0] + mesh->mNumVertices);
	}

    // Consider only first color set
    if (mesh->HasVertexColors(0))
	{
        colors = new std::vector<aiColor4t<float>>();
        colors->reserve(mesh->mNumVertices);
        colors->insert(colors->begin(),
            mesh->mColors[0], mesh->mColors[0] + mesh->mNumVertices);
    }

    //--------------------------------------------------------------------------
	// Bounding box
	boundingBox = RepoBoundingBox(mesh);

    //--------------------------------------------------------------------------
    // SHA-256 hash
    //vertexHash = hash(*vertices, boundingBox);

    //--------------------------------------------------------------------------
	// Polygon mesh outline (2D bounding rectangle in XY for the moment)
	outline = new std::vector<aiVector2t<float>>();
	boundingBox.toOutline(outline);

    //--------------------------------------------------------------------------
	// Material (always only one per mesh)
	if (mesh->mMaterialIndex < materials.size())
	{
		this->addChild(materials[mesh->mMaterialIndex]);
		materials[mesh->mMaterialIndex]->addParent(this);
	}

    //std::cerr << getVertexHash() << std::endl;
}

//------------------------------------------------------------------------------

repo::core::RepoNodeMesh::RepoNodeMesh(
	const mongo::BSONObj &obj) : RepoNodeAbstract(obj),
		vertices(NULL),
		faces(NULL),
		normals(NULL),
		outline(NULL),
        uvChannels(NULL),
        colors(NULL)
{
    //--------------------------------------------------------------------------
	// Vertices
	if (obj.hasField(REPO_NODE_LABEL_VERTICES) &&
		obj.hasField(REPO_NODE_LABEL_VERTICES_COUNT))
	{
		vertices = new std::vector<aiVector3t<float>>();
		RepoTranscoderBSON::retrieve(
			obj.getField(REPO_NODE_LABEL_VERTICES),
			obj.getField(REPO_NODE_LABEL_VERTICES_COUNT).numberInt(),
			vertices);
	}

    //--------------------------------------------------------------------------
	// Faces
	if (obj.hasField(REPO_NODE_LABEL_FACES) &&
		obj.hasField(REPO_NODE_LABEL_FACES_COUNT) &&
		obj.hasField(REPO_NODE_LABEL_FACES_BYTE_COUNT))
	{
		faces = new std::vector<aiFace>();
		retrieveFacesArray(
			obj.getField(REPO_NODE_LABEL_FACES),
			api,
			obj.getField(REPO_NODE_LABEL_FACES_BYTE_COUNT).numberInt(),
			obj.getField(REPO_NODE_LABEL_FACES_COUNT).numberInt(),
			faces);
	}

    //--------------------------------------------------------------------------
	// Normals
	if (obj.hasField(REPO_NODE_LABEL_NORMALS) &&
		obj.hasField(REPO_NODE_LABEL_VERTICES_COUNT))
	{
        normals = new std::vector<aiVector3t<float> >();
		RepoTranscoderBSON::retrieve(
			obj.getField(REPO_NODE_LABEL_NORMALS),
			obj.getField(REPO_NODE_LABEL_VERTICES_COUNT).numberInt(),
			normals);
	}

    //--------------------------------------------------------------------------
	// Polygon mesh outline (2D bounding rectangle in XY for the moment)
	//
    if (obj.hasField(REPO_NODE_LABEL_OUTLINE))
    {
        //outline = new std::vector<aiVector2D>();
        // TODO: fill in
    }

    //--------------------------------------------------------------------------
	// Bounding box
    if (obj.hasField(REPO_NODE_LABEL_BOUNDING_BOX))
    {
		std::pair<aiVector3D, aiVector3D> min_max = RepoTranscoderBSON::retrieveBBox(
            obj.getField(REPO_NODE_LABEL_BOUNDING_BOX));

		this->boundingBox.setMin(min_max.first);
		this->boundingBox.setMax(min_max.second);
    }



    //--------------------------------------------------------------------------
    // SHA-256 hash
    if (obj.hasField(REPO_NODE_LABEL_SHA256))
    {
        vertexHash = obj.getField(REPO_NODE_LABEL_SHA256).numberInt();
    }

    //--------------------------------------------------------------------------
	// UV channels
	if (obj.hasField(REPO_NODE_LABEL_UV_CHANNELS) &&
		obj.hasField(REPO_NODE_LABEL_UV_CHANNELS_BYTE_COUNT) &&
		obj.hasField(REPO_NODE_LABEL_UV_CHANNELS_COUNT))
	{
		unsigned int numberOfConcatenatedEntries =
			obj.getField(REPO_NODE_LABEL_UV_CHANNELS_COUNT).numberInt() *
			obj.getField(REPO_NODE_LABEL_VERTICES_COUNT).numberInt();

		std::vector<aiVector2t<float>> concatenated;
		RepoTranscoderBSON::retrieve(
			obj.getField(REPO_NODE_LABEL_UV_CHANNELS),
			numberOfConcatenatedEntries,
			&concatenated);

		unsigned int channelsCount =
			obj.getField(REPO_NODE_LABEL_UV_CHANNELS_COUNT).numberInt();

        unsigned int channelSize = (unsigned int) concatenated.size()/channelsCount;
		uvChannels =
			new std::vector<std::vector<aiVector3t<float>> *>(channelsCount);

		// For every uv channel present (usually only one)
		for (unsigned int i = 0; i < channelsCount; ++i)
		{
			uvChannels->at(i) = new std::vector<aiVector3t<float>>();
			uvChannels->at(i)->reserve(channelSize);

			// Copy over all per vertex UV values
			for (unsigned int j = 0; j < channelSize; ++j)
			{
				aiVector2t<float> uv = concatenated[(i * channelSize) + j];
				uvChannels->at(i)->push_back(aiVector3t<float>(uv.x, uv.y, 0));
			}
		}
	}
}

//------------------------------------------------------------------------------
//
// Destructor
//
//------------------------------------------------------------------------------
repo::core::RepoNodeMesh::~RepoNodeMesh()
{
	if (NULL != vertices)
	{
		vertices->clear();
		delete vertices;
	}

	if (NULL != faces)
	{
		faces->clear();
		delete faces;
	}

	if (NULL != normals)
	{
		normals->clear();
		delete normals;
	}

	if (NULL != outline)
	{
		outline->clear();
		delete outline;
	}

	if (NULL != uvChannels)
	{

        std::vector<std::vector<aiVector3t<float>>*>::iterator it;
        for (it = uvChannels->begin(); it != uvChannels->end(); ++it)
        {
            std::vector<aiVector3t<float>> *channel = *it;
			channel->clear();
			delete channel;
			channel = NULL;
		}
		uvChannels->clear();
		uvChannels = NULL;
	}

    if (NULL != colors){
        colors->clear();
        delete colors;
    }
}

//------------------------------------------------------------------------------
//
// Operators
//
//------------------------------------------------------------------------------

bool repo::core::RepoNodeMesh::operator==(const RepoNodeAbstract& other) const
{
    const RepoNodeMesh *otherMesh = dynamic_cast<const RepoNodeMesh*>(&other);
    return otherMesh &&
            RepoNodeAbstract::operator==(other) &&
            (std::equal(this->getVertices()->begin(),
                        this->getVertices()->end(),
                        otherMesh->getVertices()->end())) &&
            (std::equal(this->getFaces()->begin(),
                        this->getFaces()->end(),
                        otherMesh->getFaces()->end())) &&
            (std::equal(this->getNormals()->begin(),
                        this->getNormals()->end(),
                        otherMesh->getNormals()->end())) &&
            (std::equal(this->getOutline()->begin(),
                        this->getOutline()->end(),
                        otherMesh->getOutline()->end())) &&
            (this->getBoundingBox() == otherMesh->getBoundingBox()) &&
            (std::equal(this->getColors()->begin(),
                        this->getColors()->end(),
                        otherMesh->getColors()->begin())) &&
            (std::equal(this->getUVChannel(0)->begin(),
                        this->getUVChannel(0)->end(),
                        otherMesh->getUVChannel(0)->begin())) &&
            (std::equal(this->getUVChannel(1)->begin(),
                        this->getUVChannel(1)->end(),
                        otherMesh->getUVChannel(1)->end())) &&
            (std::equal(this->getUVChannel(2)->begin(),
                        this->getUVChannel(2)->end(),
                        otherMesh->getUVChannel(2)->end()));
}

//------------------------------------------------------------------------------
//
// Export
//
//------------------------------------------------------------------------------
mongo::BSONObj repo::core::RepoNodeMesh::toBSONObj() const
{
	mongo::BSONObjBuilder builder;

    //--------------------------------------------------------------------------
	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	appendDefaultFields(builder);

    //--------------------------------------------------------------------------
	// Vertices
	if (NULL != vertices && vertices->size() > 0)
		RepoTranscoderBSON::append(
			REPO_NODE_LABEL_VERTICES,
			vertices,
			builder,
			REPO_NODE_LABEL_VERTICES_BYTE_COUNT,
			REPO_NODE_LABEL_VERTICES_COUNT);

    //--------------------------------------------------------------------------
	// Faces
	if (NULL != faces && faces->size() > 0)
	{
		builder << REPO_NODE_LABEL_FACES_COUNT << (unsigned int) (faces->size());

		// In API LEVEL 1, faces are stored as
		// [n1, v1, v2, ..., n2, v1, v2...]
		std::vector<unsigned int> facesLevel1;
		for (unsigned int f = 0; f < faces->size(); ++f)
		{
			aiFace face = (*faces)[f];
			facesLevel1.push_back(face.mNumIndices);
			for (unsigned int i = 0; i < face.mNumIndices; ++i)
				facesLevel1.push_back(face.mIndices[i]);
		}

		RepoTranscoderBSON::append(
			REPO_NODE_LABEL_FACES,
			&facesLevel1,
			builder,
			REPO_NODE_LABEL_FACES_BYTE_COUNT);
	}

    //--------------------------------------------------------------------------
	// Normals
	// TODO: modify so that the empty string does not need to be passed in.
	// If "" is not used, this method calls the most generict append(T) method!
	if (NULL != normals && normals->size() > 0)
		RepoTranscoderBSON::append(
			REPO_NODE_LABEL_NORMALS,
			normals,
			builder,
			"");

    //--------------------------------------------------------------------------
	// Bounding box
	RepoTranscoderBSON::append(
		REPO_NODE_LABEL_BOUNDING_BOX,
		boundingBox.toVector(),
		builder);

    //--------------------------------------------------------------------------
    // SHA-256 hash
    if (!vertexHash.empty())
    {
		// TODO: Fix this call - needs to be fixed as int conversion is overloaded
		//builder << REPO_NODE_LABEL_SHA256 << (long unsigned int)(vertexHash);
    }

    //--------------------------------------------------------------------------
	// Outline
    if (outline && outline->size() > 0)
		RepoTranscoderBSON::append(
			REPO_NODE_LABEL_OUTLINE,
			*outline,
			builder);

    //--------------------------------------------------------------------------
	// TODO: bi/tangents, vertex colors

    //--------------------------------------------------------------------------
    // Vertex colors
    if (colors && colors->size())
        RepoTranscoderBSON::append(
                    REPO_NODE_LABEL_COLORS,
                    *colors,
                    builder);

    //--------------------------------------------------------------------------
	// UV channels
	if (NULL != uvChannels && uvChannels->size() > 0)
	{
		// Could be unsigned __int64 if BSON had such construct (the closest is only __int64)
		builder << REPO_NODE_LABEL_UV_CHANNELS_COUNT << (unsigned int)(uvChannels->size());

		std::vector<aiVector2t<float>> concatenated;

        std::vector<std::vector<aiVector3t<float>>*>::iterator it;
        for (it = uvChannels->begin(); it != uvChannels->end(); ++it)
        {
            std::vector<aiVector3t<float>> *channel = *it;

             std::vector<aiVector3t<float>>::iterator cit;
             for (cit = channel->begin(); cit != channel->end(); ++cit)
             {

                const aiVector3t<float> uv = *cit;
//		for each (std::vector<aiVector3t<float>> * channel in *uvChannels)
//			for each (const aiVector3t<float> &uv in *channel)
				concatenated.push_back(aiVector2t<float>(uv.x, uv.y));
             }
        }

		RepoTranscoderBSON::append(
			REPO_NODE_LABEL_UV_CHANNELS,
			&concatenated,
			builder,
			REPO_NODE_LABEL_UV_CHANNELS_BYTE_COUNT);
	}


	return builder.obj();
}


void repo::core::RepoNodeMesh::retrieveFacesArray(
    const mongo::BSONElement &bse,
	const unsigned int api,
	const unsigned int facesByteCount,
	const unsigned int facesCount,
    std::vector<aiFace> *faces)
{
    //--------------------------------------------------------------------------
	// TODO make use of RepoTranscoderBSON to retrieve vector of unsigned int
	if (REPO_NODE_API_LEVEL_1 == api)
	{
		faces->resize(facesCount);
		unsigned int * serializedFaces = new unsigned int[facesByteCount/sizeof(unsigned int)];
		if (NULL != faces &&
			NULL != serializedFaces &&
			facesCount > 0 &&
			bse.binDataType() == mongo::BinDataGeneral)
		{
			// Copy over all the integers
			int len = (int) facesByteCount;
			const char *binData = bse.binData(len);
			memcpy(serializedFaces, binData, facesByteCount);

			// Retrieve numbers of vertices for each face and subsequent
			// indices into the vertex array.
			// In API level 1, mesh is represented as
			// [n1, v1, v2, ..., n2, v1, v2...]
			unsigned int counter = 0;
			int mNumIndicesIndex = 0;
			while (counter < facesCount)
			{
				int mNumIndices = serializedFaces[mNumIndicesIndex];
				aiFace face;
				face.mNumIndices = mNumIndices;
				unsigned int *indices = new unsigned int[mNumIndices];
				for (int i = 0; i < mNumIndices; ++i)
					indices[i] = serializedFaces[mNumIndicesIndex + 1 + i];
				face.mIndices = indices;
				(*faces)[counter] = face;
				mNumIndicesIndex = mNumIndicesIndex + mNumIndices + 1;
				++counter;
			}
		}

		// Memory cleanup
		if (NULL != serializedFaces)
			delete [] serializedFaces;

	}
	else if (REPO_NODE_API_LEVEL_2 == api)
	{
		// TODO: triangles only
	}
	else if (REPO_NODE_API_LEVEL_3 == api)
	{
		// TODO: compression
	}
}

void repo::core::RepoNodeMesh::toAssimp(
		const std::map<const RepoNodeAbstract *, unsigned int> materialMapping,
		aiMesh * mesh) const
{
    //--------------------------------------------------------------------------
	// Name
	mesh->mName = aiString(name);

    //--------------------------------------------------------------------------
	// Faces
	if (NULL != faces && 0 < faces->size())
	{
		aiFace * facesArray = new aiFace[faces->size()];
		if (NULL != facesArray)
		{
			std::copy(faces->begin(), faces->end(), facesArray);
			mesh->mFaces = facesArray;
            mesh->mNumFaces = (unsigned int) faces->size();
			mesh->mPrimitiveTypes = 4; // TODO: work out the exact primitive type of each mesh!
		}
		else
			mesh->mNumFaces = 0;
	}
	else
		mesh->mNumFaces = 0;

    //--------------------------------------------------------------------------
	// Vertices
	// Make a copy of vertices
	aiVector3D * verticesArray = new aiVector3D[vertices->size()];
	if (NULL != verticesArray)
	{
		std::copy(vertices->begin(), vertices->end(), verticesArray);
		mesh->mVertices = verticesArray;
        mesh->mNumVertices = (unsigned int) vertices->size();
	}
	else
		mesh->mNumVertices = 0;

    //--------------------------------------------------------------------------
	// Normals
	// Make a copy of normals
	if (NULL != normals && 0 < normals->size())
	{
		aiVector3D * normalsArray = new aiVector3D[normals->size()];
		if (NULL != normalsArray)
		{
			std::copy(normals->begin(), normals->end(), normalsArray);
			mesh->mNormals = normalsArray;
		}
	}

    //--------------------------------------------------------------------------
	// Texture coordinates
	//
	// TODO: change to support U and UVW, not just UV as done now.
	if (NULL != uvChannels && 0 < uvChannels->size())
	{
		for (unsigned int i = 0; i < uvChannels->size() &&
			i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i)
		{
			aiVector3D * texCoords = new aiVector3D[vertices->size()];
			std::copy(uvChannels->at(i)->begin(), uvChannels->at(i)->end(),
				texCoords);
			mesh->mTextureCoords[i] = texCoords;
			mesh->mNumUVComponents[i] = 2; // UV
		}
	}

    //--------------------------------------------------------------------------
    // Vertex colors
    if(NULL != colors && 0 < colors->size())
    {
        aiColor4D * colorsArray = new aiColor4D[colors->size()];
        std::copy(colors->begin(), colors->end(), colorsArray);
        mesh->mColors[0] = colorsArray;
    }

    //--------------------------------------------------------------------------
	// Material index
	//
	// In assimp, mesh would be expected to have only one child.
	// If multiple children materials are found, takes the first one
	std::map<const RepoNodeAbstract *, unsigned int>::const_iterator it;

    std::set<const RepoNodeAbstract *>::iterator childrenIt;
    for (childrenIt = children.begin(); childrenIt != children.end(); ++childrenIt)
    {
		it = materialMapping.find(*childrenIt);
		if (materialMapping.end() != it)
		{
			mesh->mMaterialIndex = it->second;
			break;
		}
	}
}

//------------------------------------------------------------------------------
double repo::core::RepoNodeMesh::getFaceArea(const unsigned int& index) const
{
	double area = 0;
	const aiFace& face = faces->at(index);
	if (3 == face.mNumIndices || 4 == face.mNumIndices)
	{
		area = getTriangleArea(face, 0, 1, 2);
		if (face.mNumIndices == 4) // quadrilateral
			area +=	getTriangleArea(face, 0, 2, 3);
	}
	return area;
}

//------------------------------------------------------------------------------
double repo::core::RepoNodeMesh::getFacePerimeter(const unsigned int& index)
	const
{
	double perimeter = 0;
	const aiFace& face = faces->at(index);
	aiVector3t<float> v;
	for (unsigned int i = 0; i < face.mNumIndices; ++i)
	{
		if ((face.mNumIndices - 1) == i)
			v = vertices->at(face.mIndices[0]);
		else
			v = vertices->at(face.mIndices[i + 1]);
		perimeter += RepoVertex::distancePointToPoint<double>(
				vertices->at(face.mIndices[i]),v);
	}
	return perimeter;
}

//------------------------------------------------------------------------------
double repo::core::RepoNodeMesh::getFacesBoundaryLength(
	const unsigned int & faceIndexA,
	const unsigned int & faceIndexB) const
{
	double boundaryLength = 0;
	const aiFace & faceA = faces->at(faceIndexA);
	const aiFace & faceB = faces->at(faceIndexB);

	std::vector<repo::core::RepoVertex> commonVertices;
	for (unsigned int i = 0; i < faceA.mNumIndices; ++i)
	{
		RepoVertex a(vertices->at(faceA.mIndices[i]));
		for (unsigned int j = 0; j < faceB.mNumIndices; ++j)
		{
			if (a == RepoVertex(vertices->at(faceB.mIndices[j])))
				commonVertices.push_back(a);
		}
	}

	if (commonVertices.size() > 1)
	{
		for (unsigned int i = 0; i < commonVertices.size() - 1; ++i)
		{
			boundaryLength += RepoVertex::distancePointToPoint<double>(
				commonVertices[i], commonVertices[i+1]);
		}
	}
	return boundaryLength;
}

//------------------------------------------------------------------------------
double repo::core::RepoNodeMesh::getTriangleArea(
	const aiFace& face,
	const unsigned int& indexA,
	const unsigned int& indexB,
	const unsigned int& indexC) const
{
	double area = 0;

	if (indexA < face.mNumIndices &&
		indexB < face.mNumIndices &&
		indexC < face.mNumIndices)
	{
		const aiVector3t<float>& a = vertices->at(face.mIndices[indexA]);
		const aiVector3t<float>& b = vertices->at(face.mIndices[indexB]);
		const aiVector3t<float>& c = vertices->at(face.mIndices[indexC]);
		// area = 1/2 * || AB x AC ||
		const aiVector3D& u = b - a; // vector AB
		const aiVector3D& v = c - a; // vector AC
		const float& w1 = u.y * v.z - u.z * v.y;
		const float& w2 = u.z * v.x - u.x * v.z;
		const float& w3 = u.x * v.y - u.y * v.x;
		area = 0.5 * std::sqrt(w1 * w1 + w2 * w2 + w3 * w3);
	}
	return area;
}

//------------------------------------------------------------------------------
repo::core::RepoVertex
	repo::core::RepoNodeMesh::getFaceCentroid(unsigned int index) const
{
	RepoVertex centroid;
	const aiFace& face = faces->at(index);
	for (unsigned int i = 0; i < face.mNumIndices; ++i)
		centroid += vertices->at(face.mIndices[i]);
	centroid /= face.mNumIndices;
	return centroid;
}

std::string repo::core::RepoNodeMesh::getVertexHash()
{
    if (vertexHash.empty())
        setVertexHash();
    return vertexHash;
}

void repo::core::RepoNodeMesh::setVertexHash()
{
    pca.initialize(*vertices);

    setVertexHash(hash(pca.getUnweightedUVWVertices(), pca.getUVWBoundingBox()));

//    std::cerr << std::endl;
//    std::cerr << "------------" << std::endl;
//    std::cerr << "PCA Vertices" << std::endl;
//    std::cerr << "BB: " << pca.getUVWBoundingBox() << std::endl;
//    for (auto v : pca.getUnweightedUVWVertices())
//        std::cerr << RepoVertex(v) << std::endl;

//    std::cerr << "Vertices" << std::endl;
//    std::cerr << "BB: " << boundingBox << std::endl;
//    for (auto v : *vertices)
//        std::cerr << RepoVertex(v) << std::endl;




//    setVertexHash(hash(*vertices, boundingBox));
}

inline float fround(double n, unsigned d)
{
  unsigned p = d - (unsigned)log10(n);
  return (float)(floor(n * pow(10.0, p) + 0.5) / pow(10.0, p));
}

//------------------------------------------------------------------------------
std::string repo::core::RepoNodeMesh::hash(
        const std::vector<aiVector3t<float> >& originalVertices,
        const RepoBoundingBox& boundingBox,
        double hashDensity)
{
//    std::cerr << std::endl;
//    std::cerr << "Mesh" << std::endl;
	std::vector<hash_type> vertexHashes;


    std::set<aiVector3t<float> > verticesSet(originalVertices.begin(), originalVertices.end());
    std::vector<aiVector3t<float> > vertices(verticesSet.begin(), verticesSet.end());


//    std::sort(vertices.begin(), vertices.end(), RepoaiVertexComparator());
//    auto uniqueEnd = std::unique(vertices.begin(), vertices.end(), RepoaiVertexComparator());
//    vertices.erase(uniqueEnd, vertices.end());

    vertexHashes.resize(vertices.size());

	const aiVector3t<float> &min = boundingBox.getMin();
	const aiVector3t<float> &max = boundingBox.getMax();

	float stride_x = (max.x - min.x);
	float stride_y = (max.y - min.y);
	float stride_z = (max.z - min.z);

    for(int v_idx = 0; v_idx < vertices.size(); v_idx++)
	{
        double norm_x = (vertices.at(v_idx).x - min.x) / stride_x;
        double norm_y = (vertices.at(v_idx).y - min.y) / stride_y;
        double norm_z = (vertices.at(v_idx).z - min.z) / stride_z;

		hash_type x_coord = (hash_type)round(hashDensity * norm_x);
		hash_type y_coord = (hash_type)round(hashDensity * norm_y);
		hash_type z_coord = (hash_type)round(hashDensity * norm_z);

		hash_type vertexIndex = x_coord
			+ (hash_type)round(hashDensity * y_coord)
			+ (hash_type)round(hashDensity * hashDensity * z_coord);

		vertexHashes[v_idx] = vertexIndex;

//        if (vertices.size() < 100)
//            std::cerr << "[ " << norm_x << ", " << norm_y << ", " << norm_z << " ]";
	}

//    std::cerr << std::endl;

    std::sort(vertexHashes.begin(), vertexHashes.end());
//    std::unique(vertexHashes.begin(), vertexHashes.end());

//    for(auto v : vertexHashes)
//    {
//        std::cerr << v << " ";
//    }
//    std::cerr << std::endl;

    size_t bufSize = vertexHashes.size() * sizeof(hash_type) + sizeof(float) * 3;
    char *buf = new char[bufSize];

    memcpy(buf, (char *)(&vertexHashes[0]), vertexHashes.size() * sizeof(hash_type));
    unsigned int idx = vertexHashes.size() * sizeof(hash_type);

	/*
	stride_x = (float)(floor((double)stride_x * 10000.0) / 10000.0);
	stride_y = (float)(floor((double)stride_y * 10000.0) / 10000.0);
	stride_z = (float)(floor((double)stride_z * 10000.0) / 10000.0);
	*/

	stride_x = fround(stride_x, 3);
	stride_y = fround(stride_y, 3);
	stride_z = fround(stride_z, 3);

    *((float *)(buf + idx)) = stride_x;
    *((float *)(buf + idx + sizeof(float))) = stride_y;
    *((float *)(buf + idx + 2 * sizeof(float))) = stride_z;

//    std::cerr << stride_x << " " << stride_y << " " << stride_z << " " << sha256(std::string(buf, bufSize)) << std::endl;

    return sha256(std::string(buf, bufSize));
}
