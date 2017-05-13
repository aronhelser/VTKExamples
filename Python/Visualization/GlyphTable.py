import vtk
 
# This example uses a "glyph table" to change the shape of the 3d glyphs
# according to a scalar value. 

# NOTE: The vtkGlyph3D filter doesn't copy over scalars to the glyphs
# generated by a table like this for some reason...

# The Wavelet Source is nice for generating a test vtkImageData set
rt = vtk.vtkRTAnalyticSource()
rt.SetWholeExtent(-2,2,-2,2,0,0)
 
# Take the gradient of the only scalar 'RTData' to get a vector attribute
grad = vtk.vtkImageGradient()
grad.SetDimensionality(3)
grad.SetInputConnection(rt.GetOutputPort())
 
# Elevation just to generate another scalar attribute that varies nicely over the data range
elev = vtk.vtkElevationFilter()
# Elevation values will range from 0 to 1 between the Low and High Points
elev.SetLowPoint(-2,-2,0)
elev.SetHighPoint(2,2,0)
elev.SetInputConnection(grad.GetOutputPort())
 
# Create simple PolyData for glyph table
cs = vtk.vtkCubeSource()
cs.SetXLength(0.5)
cs.SetYLength(1)
cs.SetZLength(2)
ss = vtk.vtkSphereSource()
ss.SetRadius(0.25)
cs2 = vtk.vtkConeSource()
cs2.SetRadius(0.25)
cs2.SetHeight(0.5)

# Set up the glyph filter
glyph = vtk.vtkGlyph3D()
glyph.SetInputConnection(elev.GetOutputPort())

# Here is where we build the glyph table
# that will be indexed into according to the IndexMode
glyph.SetSource(0,cs.GetOutput())
glyph.SetSource(1,ss.GetOutput())
glyph.SetSource(2,cs2.GetOutput())

glyph.ScalingOn()
glyph.SetScaleModeToScaleByScalar()
glyph.SetVectorModeToUseVector()
glyph.OrientOn()
glyph.SetScaleFactor(1)		# Overall scaling factor
glyph.SetRange(0, 1)    	# Default is (0,1)
 
# Tell it to index into the glyph table according to scalars
glyph.SetIndexModeToScalar()

# Tell glyph which attribute arrays to use for what
glyph.SetInputArrayToProcess(0,0,0,0,'Elevation')		# scalars
glyph.SetInputArrayToProcess(1,0,0,0,'RTDataGradient')		# vectors
 
# I would call Update if I could use the scalar range to set the color map range
# glyph.Update()
 
coloring_by = 'RTData'
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(glyph.GetOutputPort())
mapper.SetScalarModeToUsePointFieldData()
mapper.SetColorModeToMapScalars()
mapper.ScalarVisibilityOn()

# GetRange() call doesn't work because attributes weren't copied to glyphs
# as they should have been...
# mapper.SetScalarRange(glyph.GetOutputDataObject(0).GetPointData().GetArray(coloring_by).GetRange())

mapper.SelectColorArray(coloring_by)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
 
ren = vtk.vtkRenderer()
ren.AddActor(actor)
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren)
iren = vtk.vtkRenderWindowInteractor()
istyle = vtk.vtkInteractorStyleTrackballCamera()
iren.SetInteractorStyle(istyle)
iren.SetRenderWindow(renWin)
ren.ResetCamera()
renWin.Render()
 
iren.Start()
