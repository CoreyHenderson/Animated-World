#include "Week9Application.h"

#define _USE_MATH_DEFINES 1
#include <math.h>

#include "StdAfx.h"


#include "Model.h"
#include <algorithm>
#include <vector>
#include <map>    
#include <fstream>
#include <sstream>
#include <iostream>


typedef struct Triple
{
	float x;
	float y;
	float z;
} Triple;

typedef struct Face
{
	int triangle[3];
	int texcoord[3];
	int normcoord[3];
} Face;


typedef struct VertTextureNormalIndex
{
	VertTextureNormalIndex(int cnt, int v_idx, int t_idx, int n_idx) : indexCnt(cnt), v(v_idx), t(t_idx), n(n_idx)
	{}
	int indexCnt;
	int v;
	int t;
	int n;
} VTN;

struct VTN_finder
{
	VTN_finder(int v_idx, int t_idx, int n_idx) : key_v(v_idx), key_t(t_idx), key_n(n_idx)
	{}

	bool operator()(const VTN& o) const
	{
		return (o.v == key_v && o.t == key_t && o.n == key_n);
	}

	const int key_v;
	const int key_t;
	const int key_n;
};


/***************************************************************************************/
// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);
	std::cout << "Loading OBJ file" << path << std::endl;
	std::cerr << "***Loading OBJ file" << path << std::endl;

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	// The model files are found in '$(ProjectDir)\resources\models\'
	std::string modelfullpath = "resources\\models\\";
	modelfullpath.append(path);

	std::ifstream file(modelfullpath);
	if (!file.is_open()) {
		printf("Impossible to open the file ! Are you in the right path ? \n");
		getchar();
		return false;
	}

	std::string line;
	while (file.good()) {

		getline(file, line);

		if (line.length() > 0)
		{
			std::stringstream sline(line);
			std::string hdr;

			sline >> hdr;	 // Consume the first token

			if (hdr.compare("v") == 0)
			{
				glm::vec3 vertex;
				sline >> vertex.x >> vertex.y >> vertex.z;
				temp_vertices.push_back(vertex);
			}
			else if (hdr.compare("vt") == 0)
			{
				glm::vec2 uv;
				sline >> uv.x >> uv.y;
				temp_uvs.push_back(uv);
			}
			else if (hdr.compare("vn") == 0)
			{
				glm::vec3 normal;
				sline >> normal.x >> normal.y >> normal.z;
				temp_normals.push_back(normal);
			}
			else if (hdr.compare("f") == 0)
			{
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				char delimeter;

				// Now, OBJ file data *MUST* cotain face data as a triple:
				// i.e.:	f  3/5/7  4/5/7 6/7/8	<-- GOOD: Vertex/UV/Normal
				// We can't accept other forms e.g.:
				//		f 1//1 2//1 3//1 4//1		<-- ERROR: Vertex//Normal
				// or
				//		f 3 4 3						<-- ERROR: Vertex Only
				//
				for (int face_cnt = 0; face_cnt < 3; face_cnt++)
				{
					sline >> vertexIndex[face_cnt];		// Should always be vertex index in first position!
					vertexIndices.push_back(vertexIndex[face_cnt]);

					// Now check for Texture & Normal indices as well...
					if (sline.peek() != '/') // We have other index values to consume
					{
						/* ERROR - can't process this form */
						file.close();
						return false;
					}
					else
					{
						sline.get(delimeter);	// Get rid of the '/'
						if (sline.peek() == '/') // Check if we had a '//' as blender does!
						{
							/* ERROR - can't process this form */
							file.close();
							return false;
						}
						else
						{
							sline >> uvIndex[face_cnt];
							uvIndices.push_back(uvIndex[face_cnt]);

							if (sline.peek() != '/') // we now have a normal to read in...
							{
								/* ERROR - can't process this form */
								file.close();
								return false;
							}
							else
							{
								sline.get(delimeter);	// Get rid of the '/'
								sline >> normalIndex[face_cnt];
								normalIndices.push_back(normalIndex[face_cnt]);
							}
						}
					}
				}
			}
		}
	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);

		// If the model had UV data, then add these to the 'out_uvs' vector
		if (uvIndices.size() > 0)
		{
			unsigned int uvIndex = uvIndices[i];
			glm::vec2 uv = temp_uvs[uvIndex - 1];
			out_uvs.push_back(uv);
		}

		// If the model had NORMAL data, then add these to the 'out_normals' vector
		if (normalIndices.size() > 0)
		{
			unsigned int normalIndex = normalIndices[i];
			glm::vec3 normal = temp_normals[normalIndex - 1];
			out_normals.push_back(normal);
		}
	}
	file.close();
	return true;
}

// Returns true iif v1 can be considered equal to v2
bool is_near(float v1, float v2) {
	return fabs(v1 - v2) < 0.01f;
}

// Searches through all already-exported vertices
// for a similar one.
// Similar = same position + same UVs + same normal
bool getSimilarVertexIndex(
	glm::vec3 & in_vertex,
	glm::vec2 & in_uv,
	glm::vec3 & in_normal,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	unsigned short & result
) {
	// Lame linear search
	for (unsigned int i = 0; i < out_vertices.size(); i++) {
		if (
			is_near(in_vertex.x, out_vertices[i].x) &&
			is_near(in_vertex.y, out_vertices[i].y) &&
			is_near(in_vertex.z, out_vertices[i].z) &&
			is_near(in_uv.x, out_uvs[i].x) &&
			is_near(in_uv.y, out_uvs[i].y) &&
			is_near(in_normal.x, out_normals[i].x) &&
			is_near(in_normal.y, out_normals[i].y) &&
			is_near(in_normal.z, out_normals[i].z)
			) {
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

void indexVBO_slow(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	// For each input vertex
	for (unsigned int i = 0; i < in_vertices.size(); i++) {

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i], out_vertices, out_uvs, out_normals, index);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			out_indices.push_back((unsigned short)out_vertices.size() - 1);
		}
	}
}

struct PackedVertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	bool operator<(const PackedVertex that) const {
		return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
	};
};

bool getSimilarVertexIndex_fast(
	PackedVertex & packed,
	std::map<PackedVertex, unsigned short> & VertexToOutIndex,
	unsigned short & result
) {
	std::map<PackedVertex, unsigned short>::iterator it = VertexToOutIndex.find(packed);
	if (it == VertexToOutIndex.end()) {
		return false;
	}
	else {
		result = it->second;
		return true;
	}
}

void indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	std::map<PackedVertex, unsigned short> VertexToOutIndex;

	// For each input vertex
	for (unsigned int i = 0; i < in_vertices.size(); i++) {

		PackedVertex packed = { in_vertices[i], in_uvs[i], in_normals[i] };


		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			unsigned short newindex = (unsigned short)out_vertices.size() - 1;
			out_indices.push_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}


Model::Model(char * filename)
{
	std::cout << "loading model: " << std::endl;

	loaddedSuccefully = true;

	std::vector<glm::vec3> newvertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	loaddedSuccefully = loadOBJ(filename, newvertices, uvs, normals);

	if (!loaddedSuccefully)
	{
		return;
	}

	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;

	indexVBO(newvertices, uvs, normals, newindices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementArrayHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, newindices.size() * sizeof(unsigned short), &newindices[0], GL_STATIC_DRAW);
}


Model::~Model(void)
{
	//delete vertices;
	//delete indices;
}


void Model::render()
{
	// Specify the name of the vertex array to bind to
	glBindVertexArray(vertexArrayHandle);

	if (renderWireframe())
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_POINT);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glPolygonMode(GL_FRONT, GL_FILL);
	}

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,				// attribute
		3,				// size
		GL_FLOAT,		// type
		GL_FALSE,		// normalized?
		0,				// stride
		(void*)0		// array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,				// attribute
		2,				// size
		GL_FLOAT,		// type
		GL_FALSE,		// normalized?
		0,				// stride
		(void*)0		// array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,				// attribute
		3,				// size
		GL_FLOAT,		// type
		GL_FALSE,		// normalized?
		0,				// stride
		(void*)0		// array buffer offset
	);

	// Specify the name of the index array to bind to
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayHandle);
	glDrawElements(
		GL_TRIANGLES,		// mode
		(GLsizei)newindices.size(),	// count
		GL_UNSIGNED_SHORT,	// type
		(void*)0			// element array buffer offset
	);

}
