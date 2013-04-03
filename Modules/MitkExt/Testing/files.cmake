set(MODULE_TESTS
  mitkAutoCropImageFilterTest.cpp
  mitkBoundingObjectCutterTest.cpp
  mitkCoreExtObjectFactoryTest
  mitkDataNodeExtTest.cpp
  mitkExternalToolsTest.cpp
  mitkMeshTest.cpp
  mitkMultiStepperTest.cpp
  mitkOrganTypePropertyTest.cpp
  # mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneFitTest.cpp
  mitkPointLocatorTest.cpp
  # mitkSegmentationInterpolationTest.cpp
  # mitkTestTemplate.cpp
  mitkUnstructuredGridTest.cpp
  mitkSimpleHistogramTest.cpp
  mitkToolManagerTest.cpp
)
set(MODULE_IMAGE_TESTS
  mitkUnstructuredGridVtkWriterTest.cpp
  mitkCompressedImageContainerTest.cpp
  mitkCylindricToCartesianFilterTest.cpp
  #mitkExtractImageFilterTest.cpp
  mitkSurfaceToImageFilterTest.cpp
)
set(MODULE_CUSTOM_TESTS
  mitkLabeledImageToSurfaceFilterTest.cpp
  mitkImageToSurfaceFilterTest.cpp
)
set(MODULE_TESTIMAGES
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
  binary.stl
  ball.stl
)
