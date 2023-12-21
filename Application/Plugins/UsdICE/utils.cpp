#include "utils.h"

int
U2ITriangulateMesh(const pxr::VtArray<int>& counts,
  const pxr::VtArray<int>& indices,
  pxr::VtArray<pxr::GfVec3i>& samples)
{
  int num_triangles = 0;
  for (auto count : counts)
  {
    num_triangles += count - 2;
  }

  samples.resize(num_triangles * 3);

  int base = 0;
  int cnt = 0;
  int prim = 0;
  for (auto count : counts)
  {
    for (int i = 1; i < count - 1; ++i)
    {
      samples[cnt++] = pxr::GfVec3i(indices[base], prim, base);
      samples[cnt++] = pxr::GfVec3i(indices[base + i], prim, base + i);
      samples[cnt++] = pxr::GfVec3i(indices[base + i + 1], prim, base + i + 1);
    }
    prim++;
    base += count;
  }
  return cnt / 3;
}

void
U2IComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& counts,
  const pxr::VtArray<int>& indices,
  const pxr::VtArray<pxr::GfVec3i>& samples,
  pxr::VtArray<pxr::GfVec3f>& normals)
{
  // we want smooth vertex normals
  normals.resize(positions.size());
  memset(normals.data(), 0.f, normals.size() * sizeof(pxr::GfVec3f));

  // first compute triangle normals
  int totalNumTriangles = samples.size() / 3;
  pxr::VtArray<pxr::GfVec3f> triangleNormals;
  triangleNormals.resize(totalNumTriangles);

  for (int i = 0; i < totalNumTriangles; ++i)
  {
    pxr::GfVec3f ab = positions[samples[i * 3 + 1][0]] - positions[samples[i * 3][0]];
    pxr::GfVec3f ac = positions[samples[i * 3 + 2][0]] - positions[samples[i * 3][0]];
    triangleNormals[i] = (ab ^ ac).GetNormalized();
  }

  // then polygons normals
  int numPolygons = counts.size();
  pxr::VtArray<pxr::GfVec3f> polygonNormals;
  polygonNormals.resize(numPolygons);
  int base = 0;
  for (int i = 0; i < counts.size(); ++i)
  {
    int numVertices = counts[i];
    int numTriangles = numVertices - 2;
    pxr::GfVec3f n(0.f, 0.f, 0.f);
    for (int j = 0; j < numTriangles; ++j)
    {
      n += triangleNormals[base + j];
    }
    polygonNormals[i] = n.GetNormalized();
    base += numTriangles;
  }

  // finaly average vertex normals  
  base = 0;
  for (int i = 0; i < counts.size(); ++i)
  {
    int numVertices = counts[i];
    for (int j = 0; j < numVertices; ++j)
    {
      normals[indices[base + j]] += polygonNormals[i];
    }
    base += numVertices;
  }

  for (auto& n : normals) n.Normalize();
}

void
U2IComputeVertexColors(const pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<pxr::GfVec3f>& colors)
{
  // we want smooth vertex normals
  colors.resize(positions.size());
  memset(colors.data(), 0.f, colors.size() * sizeof(pxr::GfVec3f));

  // set random color per vertex  
  for (int i = 0; i < colors.size(); ++i)
  {
    colors[i] = pxr::GfVec3f(
      (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX
    );
  }
}
