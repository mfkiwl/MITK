#include "mitkPolyDataGLMapper2D.h"

#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkSurfaceData.h"
#include "mitkColorProperty.h"
#include "mitkFloatProperty.h"
#include "mitkBoolProperty.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkBaseVtkMapper3D.h"

#include <vtkPolyData.h>
#include <vtkPolyDataSource.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkLinearTransform.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>

#include <itkProcessObject.h>

const mitk::BaseData *mitk::PolyDataGLMapper2D::GetInput( void )
{
    if ( this->GetNumberOfInputs() < 1 )
    {
        return 0;
    }

    return dynamic_cast<const mitk::BaseData * > ( GetData() );
}


void mitk::PolyDataGLMapper2D::Paint( mitk::BaseRenderer * renderer )
{
    if ( IsVisible( renderer ) == false )
        return ;

    // ok, das ist aus GenerateData kopiert
    mitk::BaseData::Pointer input = const_cast<mitk::BaseData*>( this->GetInput() );

    assert( input );

    bool useCellData;
    if ( dynamic_cast<mitk::BoolProperty *>( this->GetDataTreeNode() ->GetProperty( "useCellDataForColouring" ).GetPointer() ) == NULL )
        useCellData = false;
    else
        useCellData = dynamic_cast<mitk::BoolProperty *>( this->GetDataTreeNode() ->GetProperty( "useCellDataForColouring" ).GetPointer() ) ->GetBool();

    input->Update();

    vtkPolyData * vtkpolydata = this->GetVtkPolyData();
    assert( vtkpolydata );


    vtkTransform * vtktransform = GetDataTreeNode() ->GetVtkTransform();
    vtkLinearTransform * inversetransform = vtktransform->GetLinearInverse();

    Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
    PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>( worldGeometry.GetPointer() );

    if ( vtkpolydata != NULL )
    {
        Vector3D point;
        Vector3D normal;

        if ( worldPlaneGeometry.IsNotNull() )
        {
            // set up vtkPlane according to worldGeometry
            point = worldPlaneGeometry->GetPlaneView().point;
            normal = worldPlaneGeometry->GetPlaneView().normal;
            normal.normalize();
            m_Plane->SetTransform( NULL );
        }
        else
        {
            //@FIXME: does not work correctly. Does m_Plane->SetTransform really transforms a "plane plane" into a "curved plane"?
            return ;
            AbstractTransformGeometry::ConstPointer worldAbstractGeometry = dynamic_cast<const AbstractTransformGeometry*>( renderer->GetCurrentWorldGeometry2D() );
            if ( worldAbstractGeometry.IsNotNull() )
            {
                // set up vtkPlane according to worldGeometry
                point = worldAbstractGeometry->GetPlaneView().point;
                normal = worldAbstractGeometry->GetPlaneView().normal;
                normal.normalize();
                m_Plane->SetTransform( worldAbstractGeometry->GetVtkAbstractTransform() ->GetInverse() );
            }
            else
                return ;
        }

        float vp[ 3 ], vnormal[ 3 ];
        vec2vtk( point, vp );
        vec2vtk( normal, vnormal );

        //normally, we would need to transform the surface and cut the transformed surface with the cutter.
        //This might be quite slow. Thus, the idea is, to perform an inverse transform of the plane instead.
        //@todo It probably does not work for scaling operations yet:scaling operations have to be
        //dealed with after the cut is performed by scaling the contour.
        inversetransform->TransformPoint( vp, vp );
        inversetransform->TransformNormalAtPoint( vp, vnormal, vnormal );

        m_Plane->SetOrigin( vp );
        m_Plane->SetNormal( vnormal );

        // set data into cutter
        m_Cutter->SetInput( vtkpolydata );
        //    m_Cutter->GenerateCutScalarsOff();
        //    m_Cutter->SetSortByToSortByCell();

        // calculate the cut
        m_Cutter->Update();

        // fetch geometry
        mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
        assert( displayGeometry );
        //  float toGL=displayGeometry->GetSizeInDisplayUnits().y;

        //apply color and opacity read from the PropertyList
        ApplyProperties( renderer );

        // travers the cut contour
        vtkPolyData * contour = m_Cutter->GetOutput();

        vtkPoints *vpoints = contour->GetPoints();
        vtkCellArray *vpolys = contour->GetLines();
        vtkPointData *vpointdata = contour->GetPointData();
        vtkDataArray* vscalars = vpointdata->GetScalars();

        vtkCellData *vcelldata = contour->GetCellData();
        vtkDataArray* vcellscalars = vcelldata->GetScalars();

        int i, numberOfCells = vpolys->GetNumberOfCells();

        Point3D p;
        Point2D p2d, last, first;

        vpolys->InitTraversal();
        for ( i = 0;i < numberOfCells;++i )
        {
            int *cell, cellSize;

            vpolys->GetNextCell( cellSize, cell );
            vpoints->GetPoint( cell[ 0 ], vp );

            //take transformation via vtktransform into account
            vtktransform->TransformPoint( vp, vp );

            vtk2vec( vp, p );

            //convert 3D point (in mm) to 2D point on slice (also in mm)
            worldGeometry->Map( p, p2d );

            //convert point (until now mm and in worldcoordinates) to display coordinates (units )
            displayGeometry->MMToDisplay( p2d, p2d );
            //   p2d.y=toGL-p2d.y;
            first = last = p2d;



            //      glColor3f(1.0f,1.0f,0.0f);
            int j;
            for ( j = 1;j < cellSize;++j )
            {
                vpoints->GetPoint( cell[ j ], vp );
                //take transformation via vtktransform into account
                vtktransform->TransformPoint( vp, vp );

                vtk2vec( vp, p );

                //convert 3D point (in mm) to 2D point on slice (also in mm)
                worldGeometry->Map( p, p2d );

                //convert point (until now mm and in worldcoordinates) to display coordinates (units )
                displayGeometry->MMToDisplay( p2d, p2d );

                //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
                //    p2d.y=toGL-p2d.y;


                if ( useCellData )
                {  // color each cell according to cell data
                    float * color;
                    float v = vcellscalars->GetComponent( i, 0 );
                    color = m_LUT->GetColor( vcellscalars->GetComponent( i, 0 ) );
                    glColor3f( color[ 0 ], color[ 1 ], color[ 2 ] );
                }

                //draw the line
                glBegin ( GL_LINE_LOOP );

                glVertex2f( last.x, last.y );
                glVertex2f( p2d.x, p2d.y );
                glVertex2f( last.x, last.y );
                glEnd ();

                last = p2d;
            }
        }
    }
}


vtkPolyData* mitk::PolyDataGLMapper2D::GetVtkPolyData( void )
{
    mitk::DataTreeNode::Pointer node = this->GetDataTreeNode();
    if ( node.IsNull() )
        return NULL;

    mitk::BaseVtkMapper3D::Pointer mitkMapper = dynamic_cast< mitk::BaseVtkMapper3D* > ( node->GetMapper( 2 ) );
    if ( mitkMapper.IsNull() )
        return NULL;

    ((itk::ProcessObject*)mitkMapper.GetPointer())->Update();

    vtkActor* actor = dynamic_cast<vtkActor*>( mitkMapper->GetProp() );

    if ( actor == NULL )
        return NULL;

    vtkPolyDataMapper* mapper = dynamic_cast<vtkPolyDataMapper*>( actor->GetMapper() );
    if ( mapper == NULL )
        return NULL;

    return mapper->GetInput();
}


bool mitk::PolyDataGLMapper2D::IsConvertibleToVtkPolyData()
{
    mitk::DataTreeNode::Pointer node = this->GetDataTreeNode();
    if ( node.IsNull() )
        return false;

    mitk::BaseVtkMapper3D::Pointer mitkMapper = dynamic_cast< mitk::BaseVtkMapper3D* > ( node->GetMapper( 2 ) );
    if ( mitkMapper.IsNull() )
        return false;
    
    ((itk::ProcessObject*)mitkMapper.GetPointer())->Update();

    vtkActor* actor = dynamic_cast<vtkActor*>( mitkMapper->GetProp() );

    if ( actor == NULL )
        return false;

    vtkPolyDataMapper* mapper = dynamic_cast<vtkPolyDataMapper*>( actor->GetMapper() );
    if ( mapper == NULL )
        return false;

    return true;
}


void mitk::PolyDataGLMapper2D::Update()
{}

mitk::PolyDataGLMapper2D::PolyDataGLMapper2D()
{
  m_Plane  = vtkPlane::New();
  m_Cutter = vtkCutter::New();

  m_Cutter->SetCutFunction(m_Plane);
  m_Cutter->GenerateValues(1,0,1);

  m_LUT = vtkLookupTable::New();
  m_LUT->SetTableRange(0,255);
  m_LUT->SetNumberOfColors(255);
  m_LUT->SetRampToLinear ();
  m_LUT->Build();
}
    
mitk::PolyDataGLMapper2D::~PolyDataGLMapper2D()
{
    
}

