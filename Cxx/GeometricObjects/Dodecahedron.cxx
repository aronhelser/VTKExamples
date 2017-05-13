#include "vtkSmartPointer.h"
#include "vtkVersion.h"

#include "vtkPolyhedron.h"
#include "vtkPolyData.h"

#include "vtkPolyDataMapper.h"
#include "vtkCamera.h"
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

vtkSmartPointer<vtkPolyhedron> MakeDodecahedron();

int main (int, char *[])
{
  vtkSmartPointer<vtkPolyhedron> dodecahedron = MakeDodecahedron();

  // Visualize
  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
  mapper->SetInput(dodecahedron->GetPolyData());
#else
  mapper->SetInputData(dodecahedron->GetPolyData());
#endif
  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(.2, .3, .4);
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(30);

  renderer->ResetCamera();

  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

vtkSmartPointer<vtkPolyhedron>MakeDodecahedron()
{
  vtkSmartPointer<vtkPolyhedron> aDodecahedron =
    vtkSmartPointer<vtkPolyhedron>::New();

  for (int i = 0; i < 20; ++i)
  {
    aDodecahedron->GetPointIds()->InsertNextId(i);
  }

  aDodecahedron->GetPoints()->InsertNextPoint(1.21412,    0,          1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(0.375185,   1.1547,     1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.982247,  0.713644,   1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.982247,  -0.713644,  1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(0.375185,   -1.1547,    1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(1.96449,    0,          0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(0.607062,   1.86835,    0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(-1.58931,   1.1547,     0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(-1.58931,   -1.1547,    0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(0.607062,   -1.86835,   0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(1.58931,    1.1547,     -0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.607062,  1.86835,    -0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(-1.96449,   0,          -0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.607062,  -1.86835,   -0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(1.58931,    -1.1547,    -0.375185);
  aDodecahedron->GetPoints()->InsertNextPoint(0.982247,   0.713644,   -1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.375185,  1.1547,     -1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(-1.21412,   0,          -1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(-0.375185,  -1.1547,    -1.58931);
  aDodecahedron->GetPoints()->InsertNextPoint(0.982247,   -0.713644,  -1.58931);

  vtkIdType faces[73] =
    {12,                   // number of faces
     5, 0, 1, 2, 3, 4,     // number of ids on face, ids
     5, 0, 5, 10, 6, 1,
     5, 1, 6, 11, 7, 2,
     5, 2, 7, 12, 8, 3,
     5, 3, 8, 13, 9, 4,
     5, 4, 9, 14, 5, 0,
     5, 15, 10, 5, 14, 19,
     5, 16, 11, 6, 10, 15,
     5, 17, 12, 7, 11, 16,
     5, 18, 13, 8, 12, 17,
     5, 19, 14, 9, 13, 18,
     5, 19, 18, 17, 16, 15};

  aDodecahedron->SetFaces(faces);
  aDodecahedron->Initialize();

  return aDodecahedron;
}
