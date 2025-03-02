#include <vtkSmartPointer.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSphereSource.h>

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkStaticPointLocator.h>

#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtksys/SystemTools.hxx>

#include <array>
#include <ostream>
#include <iterator>

namespace
{
vtkSmartPointer<vtkPolyData> ReadPolyData(const char *fileName);
}

template <class T, std::size_t N>
ostream &operator<<(ostream &o, const std::array<T, N> &arr)
{
  copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, ", "));
  return o;
}

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " shark.ply [number of radii] " << std::endl;
    return EXIT_FAILURE;
  }

  int numberOfRadii = 5;
  if (argc > 2)
  {
    numberOfRadii = std::atoi(argv[2]);
  }
  // Read the polydata
  vtkSmartPointer<vtkPolyData> polyData = ReadPolyData(argc > 1 ? argv[1] : "");

  // Compute bounds and range
  std::array<double, 6> bounds;
  polyData->GetBounds(bounds.data());
  std::cout << "Bounds: " << bounds << std::endl;
  std::array<double, 3> range;
  range[0] = bounds[1] - bounds[0];
  range[1] = bounds[3] - bounds[2];
  range[2] = bounds[5] - bounds[4];
  std::cout << "Range: " << range << std::endl;

  double maxRange =
    std::max({range[0], range[1], range[2]});
  double minRange =
    std::min({range[0], range[1], range[2]});

  // Define a sphere at one edge of bounding box
  auto sphereSource =
    vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(
    range[0] / 2.0 + bounds[0],
    range[1] / 2.0 + bounds[2],
    bounds[5]);
  sphereSource->SetRadius(minRange);
  sphereSource->SetPhiResolution(31);
  sphereSource->SetThetaResolution(31);
  sphereSource->SetStartPhi(90.0);
  sphereSource->Update();

  // Initialize the locator
  auto pointTree =
    vtkSmartPointer<vtkStaticPointLocator>::New();
  pointTree->SetDataSet(polyData);
  pointTree->BuildLocator();

  // Compute the radius for each call to FindPointsWithinRadius
  std::vector<double> radii;
  double radiiStart = .25 * sphereSource->GetRadius();
  double radiiEnd = 1.0  * sphereSource->GetRadius();
  double radiiDelta = (radiiEnd - radiiStart) / (numberOfRadii - 1);

  for (int r = 0; r < numberOfRadii; ++r)
  {
    radii.push_back(radiiStart + radiiDelta * r);
  }

  // Create an array to hold the scalar point data
  auto scalars =
    vtkSmartPointer<vtkDoubleArray> ::New();
  scalars->SetNumberOfComponents(1);
  scalars->SetNumberOfTuples(polyData->GetNumberOfPoints());
  scalars->FillComponent(0, 0.0);

  // Process each radii from largest to smallest
  for (std::vector<double>::reverse_iterator rIter = radii.rbegin(); 
       rIter != radii.rend();
       ++rIter)
  {
    auto result =
      vtkSmartPointer<vtkIdList>::New();
    pointTree->FindPointsWithinRadius(
      *rIter,
      sphereSource->GetCenter(),
      result);
    vtkIdType k = result->GetNumberOfIds();
    std::cout << k << " points within "
              << *rIter
              << " of "
              << sphereSource->GetCenter()[0] << ", "
              << sphereSource->GetCenter()[1] << ", "
              << sphereSource->GetCenter()[2] << std::endl;
    // Store the distance in the points withnin the current radius
    for(vtkIdType i = 0; i < k; i++)
    {
      vtkIdType point_ind = result->GetId(i);
      double p[3];
      scalars->SetTuple1(point_ind, *rIter);
    }
  }
  polyData->GetPointData()->SetScalars(scalars);

  // Visualize
  auto colors =
    vtkSmartPointer<vtkNamedColors>::New();

  auto renderer =
    vtkSmartPointer<vtkRenderer>::New();

  auto lut =
    vtkSmartPointer<vtkLookupTable>::New();
  lut->SetHueRange(.667, 0.0);
  lut->SetNumberOfTableValues(radii.size() + 1);
  lut->SetRange(*radii.begin(), *radii.rbegin());
  lut->Build();

  // Create a transluscent sphere for each radii
  if (radii.size() < 6)
  {
    int r = 0;
    for (std::vector<double>::reverse_iterator rIter = radii.rbegin(); 
         rIter != radii.rend();
         ++rIter)
    {
      auto radiiSource =
        vtkSmartPointer<vtkSphereSource>::New();
      radiiSource->SetPhiResolution(31);
      radiiSource->SetThetaResolution(31);
      radiiSource->SetStartPhi(90.0);
      radiiSource->SetRadius(*rIter);
      radiiSource->SetCenter(
        range[0] / 2.0 + bounds[0],
        range[1] / 2.0 + bounds[2],
        bounds[5]);

      auto radiiMapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
      radiiMapper->SetInputConnection(radiiSource->GetOutputPort());

      auto backProp =
        vtkSmartPointer<vtkProperty>::New();
      backProp->SetDiffuseColor(colors->GetColor3d("LightGrey").GetData());

      auto radiiActor =
        vtkSmartPointer<vtkActor>::New();
      radiiActor->SetMapper(radiiMapper);
      radiiActor->GetProperty()->SetDiffuseColor(colors->GetColor3d("White").GetData());
      radiiActor->GetProperty()->SetOpacity(.1);
      radiiActor->SetBackfaceProperty(backProp);

      renderer->AddActor(radiiActor);
    }
  }

  // Display the original poly data
  auto mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputData(polyData);
  mapper->SetLookupTable(lut);
  mapper->SetScalarRange(*radii.begin(), *radii.rbegin());

  auto actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Crimson").GetData());
  actor->GetProperty()->SetInterpolationToFlat();

  auto renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetSize(640, 480);
  renderWindow->AddRenderer(renderer);

  auto renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BurlyWood").GetData());
  renderer->UseHiddenLineRemovalOn();

  renderWindow->Render();

  // Pick a good view
  renderer->GetActiveCamera()->Azimuth(-30);
  renderer->GetActiveCamera()->Elevation(30);
  renderer->GetActiveCamera()->Dolly(1.25);
  renderer->ResetCameraClippingRange();
  renderWindow->Render();

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

namespace
{
vtkSmartPointer<vtkPolyData> ReadPolyData(const char *fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));
  if (extension == ".ply")
  {
    auto reader =
      vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    auto reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    auto reader =
      vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    auto reader =
      vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    auto reader =
      vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    auto reader =
      vtkSmartPointer<vtkBYUReader>::New();
    reader->SetGeometryFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    auto source =
      vtkSmartPointer<vtkSphereSource>::New();
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}
}
