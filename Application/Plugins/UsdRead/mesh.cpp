#include "mesh.h"
#include "utils.h"
#include <xsi_customoperator.h>
#include <xsi_operatorcontext.h>
#include <pxr/usd/usdGeom/mesh.h>

U2XMesh::U2XMesh(const pxr::UsdPrim& prim, U2XPrim* parent)
  : U2XPrim(prim, parent)
{

}

U2XMesh::~U2XMesh()
{

}

void U2XMesh::Init()
{
  pxr::UsdTimeCode timeCode(pxr::UsdTimeCode::Default());
  GetVisibility(timeCode, true);
  GetXform(timeCode);

  pxr::UsdGeomMesh mesh(_prim);
  pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
  pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
  pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

  _pointsVarying = false;
  if (pointsAttr.GetNumTimeSamples() > 1)_pointsVarying = true;
  _topoVarying = false;
  if (faceVertexIndicesAttr.GetNumTimeSamples() > 1)_topoVarying = true;

  pointsAttr.Get(&_points, timeCode);
  if(!_points.size()) pointsAttr.Get(&_points, pxr::UsdTimeCode::EarliestTime());
  faceVertexCountsAttr.Get(&_counts, timeCode);
  faceVertexIndicesAttr.Get(&_indices, timeCode);

  U2XTriangulateMesh(_counts, _indices, _samples);

  _topology.numElements = _samples.size();
  _topology.type = U2XTopology::Type::TRIANGLES;
  _topology.samples = (const int*)&_samples[0][0];
  _vao.SetTopologyPtr(&_topology);

  _vao.SetNeedReallocate(true);
  _vao.SetNumElements(_samples.size());
  _vao.SetHaveChannel(CHANNEL_POSITION);
  _vao.SetHaveChannel(CHANNEL_NORMAL);

  size_t hash;
  // points
  U2XVertexBuffer* pointsBuffer =
    _vao.CreateBuffer(
      CHANNEL_POSITION,
      _points.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_POSITION, pointsBuffer);
  pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());
  pointsBuffer->ComputeHash((const char*)&_points[0]);

  // normals
  pxr::UsdAttribute normalsAttr = mesh.GetNormalsAttr();

  U2XComputeVertexNormals(_points, _counts, _indices, _samples, _normals);
  U2XVertexBuffer* normalsBuffer =
    _vao.CreateBuffer(
      CHANNEL_NORMAL,
      _normals.size(),
      _samples.size(),
      INTERPOLATION_VERTEX);
  _vao.SetBuffer(CHANNEL_NORMAL, normalsBuffer);
  normalsBuffer->SetRawInputDatas((const char*)&_normals[0], _normals.size());
  normalsBuffer->ComputeHash((const char*)&_normals[0]);

  // colors
  pxr::TfToken colorAttrName("displayColor");
  U2XAttributeType attrType = HasAttribute(colorAttrName);
  
  if (attrType == ATTR_PRIMVAR)
  {
    U2XAttribute colorAttribute = CreateAttribute(colorAttrName, attrType);
    const pxr::UsdAttribute& attr = colorAttribute.Get();

    attr.Get<pxr::VtArray<pxr::GfVec3f>>(&_colors, pxr::UsdTimeCode::Default());
    if (_colors.size())
    {
      _vao.SetHaveChannel(CHANNEL_COLOR);

      U2XVertexBuffer* colorsBuffer =
        _vao.CreateBuffer(
          CHANNEL_COLOR,
          _colors.size(),
          _samples.size(),
          colorAttribute.GetInterpolation());
      _vao.SetBuffer(CHANNEL_COLOR, colorsBuffer);
      colorsBuffer->SetRawInputDatas((const char*)&_colors[0][0], _colors.size());
      colorsBuffer->ComputeHash((const char*)&_colors[0][0]);
    }
    /*
    U2XAttribute colorAttribute = CreateAttribute(colorAttrName, attrType);

    const VtValue& values = colorAttribute.Get(pxr::UsdTimeCode::EarliestTime());
    if (values.IsHolding<pxr::VtArray<pxr::GfVec3f>>())
    {
      const pxr::VtArray<pxr::GfVec3f>& colors = values.UncheckedGet<pxr::VtArray<GfVec3f>>();

      if (colors.size())
      {
        _vao.SetHaveChannel(CHANNEL_COLOR);

        U2XVertexBuffer* colorsBuffer =
          _vao.CreateBuffer(
            CHANNEL_COLOR,
            colors.size(),
            _samples.size(),
            colorAttribute.GetInterpolation());
        _vao.SetBuffer(CHANNEL_COLOR, colorsBuffer);
        colorsBuffer->SetRawInputDatas((const char*)&colors[0][0], colors.size());
        colorsBuffer->ComputeHash((const char*)&colors[0][0]);
      }
      */

  }
}

void U2XMesh::Term()
{

}

void U2XMesh::Update(double t, bool forceUpdate)
{
  
  pxr::UsdTimeCode timeCode(t);
  GetVisibility(timeCode, forceUpdate);
  bool pointsPositionUpdated = false;
  bool topoUpdated = false;

  pxr::UsdGeomMesh mesh(_prim);
  size_t hash;
  // topo
  if (_topoVarying)
  {
    pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();
    pxr::UsdAttribute faceVertexIndicesAttr = mesh.GetFaceVertexIndicesAttr();

    faceVertexCountsAttr.Get(&_counts, timeCode);
    faceVertexIndicesAttr.Get(&_indices, timeCode);

    U2XTriangulateMesh(_counts, _indices, _samples);

    _topology.numElements = _samples.size();
    _topology.samples = (const int*)&_samples[0][0];
    _vao.SetTopologyPtr(&_topology);

    _vao.SetNeedReallocate(true);
    _vao.SetNumElements(_samples.size());
    pointsPositionUpdated = true;
    topoUpdated = true;
  }

  // points
  if (_pointsVarying || topoUpdated)
  {
    pxr::UsdAttribute pointsAttr = mesh.GetPointsAttr();
    
    pointsAttr.Get(&_points, timeCode);

    U2XVertexBuffer* pointsBuffer = _vao.GetBuffer(CHANNEL_POSITION);
    if (_points.size() != pointsBuffer->GetNumInputElements())
      pointsBuffer->SetNeedReallocate(true);
    pointsBuffer->SetRawInputDatas((const char*)&_points[0], _points.size());
    hash = pointsBuffer->GetHash();
    if (hash != pointsBuffer->ComputeHash((const char*)&_points[0]))
    {
      pointsBuffer->SetNeedUpdate(true);
      pointsPositionUpdated = true;
    }
    if (topoUpdated)pointsBuffer->SetNumOutputElements(_vao.GetNumElements());

    if (pointsPositionUpdated || topoUpdated)
    {
      // normals
      U2XComputeVertexNormals(_points, _counts, _indices, _samples, _normals);
      U2XVertexBuffer* normalsBuffer = _vao.GetBuffer(CHANNEL_NORMAL);
      if (_normals.size() != normalsBuffer->GetNumInputElements())
        normalsBuffer->SetNeedReallocate(true);
      normalsBuffer->SetRawInputDatas((const char*)&_normals[0], _normals.size());
      hash = normalsBuffer->GetHash();

      if (hash != normalsBuffer->ComputeHash((const char*)&_normals[0]))
      {
        normalsBuffer->SetNeedUpdate(true);
      }
      if (topoUpdated)normalsBuffer->SetNumOutputElements(_vao.GetNumElements());
    }

    if (topoUpdated || _colorsVarying)
    {
      // colors
      U2XComputeVertexColors(_points, _colors);
      U2XVertexBuffer* colorsBuffer = _vao.GetBuffer(CHANNEL_COLOR);
      if (_colors.size() != colorsBuffer->GetNumInputElements())
        colorsBuffer->SetNeedReallocate(true);
      colorsBuffer->SetRawInputDatas((const char*)&_colors[0], _colors.size());
      hash = colorsBuffer->GetHash();

      if (hash != colorsBuffer->ComputeHash((const char*)&_colors[0]))
      {
        colorsBuffer->SetNeedUpdate(true);
      }
      if (topoUpdated)colorsBuffer->SetNumOutputElements(_vao.GetNumElements());
    }
  }
  
  _vao.UpdateState();
}

void U2XMesh::Prepare()
{
  if(_vao.GetNeedReallocate())_vao.Reallocate();
  if(_vao.GetNeedUpdate())_vao.Populate();
}

void U2XMesh::Draw()
{
  _vao.Draw();
}

XSIPLUGINCALLBACK CStatus UsdMeshGenerator_Define(CRef& in_ctxt)
{
  Context ctxt(in_ctxt);
  CustomOperator op;
  Parameter param;
  CRef paramDef;
  Factory factory = Application().GetFactory();
  op = ctxt.GetSource();
  /*
  paramDef = oFactory.CreateParamDef(L"CacheFile", CValue::siString, L"");
  op.AddParameter(paramDef, param);
  paramDef = oFactory.CreateParamDef(L"CacheStartFrame", CValue::siInt4, siPersistable | siReadOnly, L"CacheStartFrame", L"CacheStartFrame", 1l, -10000l, 10000l, -1000l, 1000l);
  op.AddParameter(paramDef, param);
  paramDef = oFactory.CreateParamDef(L"CacheEndFrame", CValue::siInt4, siPersistable | siReadOnly, L"CacheEndFrame", L"CacheEndFrame", 100l, -10000l, 10000l, -1000l, 1000l);
  op.AddParameter(paramDef, param);
  oPDef = oFactory.CreateParamDef(L"TimeOffset", CValue::siDouble, siPersistable | siAnimatable, L"TimeOffset", L"TimeOffset", 0l, -10000l, 10000l, -100l, 100l);
  op.AddParameter(paramDef, param);
  */
  op.PutAlwaysEvaluate(true);
  op.PutDebug(0);
  return CStatus::OK;
}

CStatus UsdMeshGeneraror_Update(OperatorContext& in_ctxt)
{
  /*
  CustomOperator oOp = in_ctxt.GetSource();

  // Get Inputs
  CString inFilePath(in_ctxt.GetParameterValue(L"CacheFile"));
  inFilePath = CUtils::ResolvePath(inFilePath);
  bool fExist = FileExists(inFilePath.GetAsciiString());

  CTime T = in_ctxt.GetTime();
  int currentFrame = int(T);

  if (fExist)
  {
    int timeOffset(in_ctxt.GetParameterValue(L"TimeOffset"));
    if (_reader._filename != inFilePath.GetAsciiString() || !_reader._file.is_open())
    {
      bool isopen = _reader.OpenFile(inFilePath.GetAsciiString());
      if (!isopen)return CStatus::InvalidArgument;

      bool isvalid = _reader.GetHeader();
      if (!isvalid)return CStatus::InvalidArgument;
    }

    currentFrame += timeOffset;
    currentFrame -= _reader._startframe;
    _reader.ReadFrame(currentFrame);

    _vertices.Clear();
    _polygons.Clear();

    for (int i = 0; i < _reader._nbv; i++)
    {
      _vertices.Add(CVector3(_reader._vertices[i * 3], _reader._vertices[i * 3 + 1], _reader._vertices[i * 3 + 2]));
    }

    int offset = 0;
    for (int i = 0; i < _reader._nbf; i++)
    {
      _polygons.Add(_reader._faceverticescount[i]);
      for (int j = 0; j < _reader._faceverticescount[i]; j++)
      {
        _polygons.Add(_reader._facevertices[offset + j]);
      }
      offset += _reader._faceverticescount[i];
    }

    Primitive output = in_ctxt.GetOutputTarget();
    PolygonMesh(output.GetGeometry()).Set(_vertices, _polygons);
  }
  else
  {
    Application().LogMessage(L"File " + inFilePath + L" DOESN'T exist !!");
  }
  */

  return CStatus::OK;
}

/*
MStatus stat;
  if(plug != outmesh)return MS::kUnknownParameter;

  MDataHandle hInput;
  hInput = data.inputValue(currenttime);
  int currentframe = (int)hInput.asFloat();

  hInput = data.inputValue(timeoffset);
  int timeoffset = hInput.asInt();

  hInput = data.inputValue(file);
  MString filename = hInput.asString();
  bool fExist = FileExists(filename);

  if(fExist)
  {
    if(reader->_filename != filename.asChar() || !reader->_file.is_open())
    {
      bool isopen = reader->OpenFile(filename.asChar());
      if(!isopen)return MS::kInvalidParameter;

      bool isvalid = reader->GetHeader();
      if(!isvalid)return MS::kInvalidParameter;
    }

    currentframe +=timeoffset;
    currentframe -= reader->_startframe;
    reader->ReadFrame(currentframe);

    MPointArray vertices;
    MVectorArray normals;
    MIntArray faceverticescount;
    MIntArray facevertices;

    for(int a=0;a<reader->_nbv;a++)
    {
      vertices.append(MPoint(reader->_vertices[a*3],reader->_vertices[a*3+1],reader->_vertices[a*3+2]));
      normals.append(MVector(reader->_normals[a*3],reader->_normals[a*3+1],reader->_normals[a*3+2]));
    }

    int fverticessum = 0;
    for(int a=0;a<reader->_nbf;a++)
    {
      fverticessum+=reader->_faceverticescount[a];
      faceverticescount.append(reader->_faceverticescount[a]);
    }

    for(int a=0;a<fverticessum;a++)
    {
      facevertices.append(reader->_facevertices[a]);
    }

    // get outMesh
    MDataHandle outputValueDataHandle = data.outputValue( outmesh );

    MFnMeshData outputDataCreator;
    MObject newOutputMeshData = outputDataCreator.create(&stat);

    MFnMesh mesh = newOutputMeshData;
    mesh.create(reader->_nbv, reader->_nbf, vertices, faceverticescount, facevertices, newOutputMeshData,&stat);

    // set outMesh
    outputValueDataHandle.set ( newOutputMeshData );
    data.setClean(plug);

    return MS::kSuccess;
  }
  else
  {
    return MS::kInvalidParameter;
  }
  */

  //////////////////////////////////////////////////////////////////////////////
  // Entry Points
  //////////////////////////////////////////////////////////////////////////////

XSIPLUGINCALLBACK CStatus UsdMeshGenerator_Init(CRef& in_ctx)
{
  Context ctx(in_ctx);
  U2XMesh* mesh = new U2XMesh();
  ctx.PutUserData((CValue::siPtrType)mesh);
  CustomOperator oOp = ctx.GetSource();
  return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus UsdMeshGenerator_Term(CRef& in_ctx)
{
  Context ctx(in_ctx);
  CValue::siPtrType pUserData = ctx.GetUserData();
  U2XMesh* mesh = (U2XMesh*)pUserData;
  delete mesh;
  return CStatus::OK; 
}

XSIPLUGINCALLBACK CStatus UsdMeshGenerator_Update(CRef& in_ctx)
{
  OperatorContext ctx(in_ctx);
  CValue::siPtrType pUserData = ctx.GetUserData();
  U2XMesh* mesh = (U2XMesh*)pUserData;
  CTime time;
  mesh->Update(time.GetTime(), true);
  return CStatus::OK;
}