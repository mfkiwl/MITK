#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkPositionEvent.h"
#include "mitkInteractionConst.h"
#include <itkCommand.h>

//##ModelId=3E189B1D008D
mitk::SliceNavigationController::SliceNavigationController(const char * type) 
  : BaseController(type), m_ViewDirection(Transversal), m_BlockUpdate(false), m_CreatedWorldGeometry(NULL), m_InputWorldGeometry(NULL)
{
  itk::SimpleMemberCommand<SliceNavigationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  timeStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
#ifdef WIN32
  sliceStepperChangedCommand->SetCallbackFunction(this, SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  SliceNavigationController::SendTime);
#else
  sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  &SliceNavigationController::SendTime);
#endif

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
  m_Time->AddObserver(itk::ModifiedEvent(),  timeStepperChangedCommand);
  ConnectGeometryEvents(this);
}

//##ModelId=3E189B1D00BF
mitk::SliceNavigationController::~SliceNavigationController()
{

}

void mitk::SliceNavigationController::Update()
{
  if(m_BlockUpdate)
    return;
  m_BlockUpdate = true;
  if(m_LastUpdateTime<GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    if(m_InputWorldGeometry.IsNotNull())
    {
      // compute bounding box with respect to first images geometry
      //mitk::BoundingBox::ConstPointer boundingBox = aGeometry3D->GetBoundingBox();
      //const_cast<mitk::BoundingBox*>(boundingBox.GetPointer())->ComputeBoundingBox();
      //const mitk::BoundingBox::BoundsArrayType bounds = boundingBox->GetBounds();
      const mitk::BoundingBox::BoundsArrayType bounds = m_InputWorldGeometry->GetBoundingBox()->GetBounds();

      Vector3D dimensionVec;
      Vector3D  origin, right, bottom;
      if ( (m_ViewDirection == Transversal) || (m_ViewDirection == Original))
      {  
        origin = Vector3f(bounds[0],bounds[3],bounds[5]);
        right = Vector3f(bounds[1],bounds[3],bounds[5]);
        bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
        dimensionVec.set(0,0,bounds[4]-bounds[5]);
      }
      else if (m_ViewDirection == Frontal)
      {  
        origin = Vector3f(bounds[0],bounds[2],bounds[4]);
        right = Vector3f(bounds[1],bounds[2],bounds[4]);
        bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
        dimensionVec.set(0,bounds[3]-bounds[2],0);
      }
      // init sagittal view
      else 
      {  
        origin = Vector3f(bounds[0],bounds[2],bounds[4]);
        right = Vector3f(bounds[0],bounds[3],bounds[4]);
        bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
        dimensionVec.set(bounds[1]-bounds[0],0,0);
      }

      m_InputWorldGeometry->MMToUnits(dimensionVec, dimensionVec);

      m_Slice->SetSteps((int)dimensionVec.length()+1.0);
      m_Slice->SetPos(0);

      // initialize the viewplane
      mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

      //@FIXME: ohne den Pointer-Umweg meckert gcc  
      mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
      planegeometry->SetPlaneView(*view);

      mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
      slicedWorldGeometry->Initialize(m_Slice->GetSteps());

      slicedWorldGeometry->SetGeometry2D(planegeometry, 0);
      slicedWorldGeometry->SetDirectionVector(dimensionVec);
      Vector3D spacing(1.0,1.0,1.0); 
      slicedWorldGeometry->SetSpacing(spacing);
      slicedWorldGeometry->SetEvenlySpaced();

      // initialize TimeSlicedGeometry
      m_CreatedWorldGeometry = TimeSlicedGeometry::New();
      const TimeSlicedGeometry* worldTimeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(m_InputWorldGeometry.GetPointer());
      if(worldTimeSlicedGeometry==NULL)
      {
        m_CreatedWorldGeometry->Initialize(1);
        m_Time->SetSteps(0);
        m_Time->SetPos(0);
      }
      else
      {
        m_BlockUpdate = true;
        m_Time->SetSteps(worldTimeSlicedGeometry->GetTimeSteps());
        m_Time->SetPos(0);
        m_BlockUpdate = false;

        m_CreatedWorldGeometry->Initialize(worldTimeSlicedGeometry->GetTimeSteps());
        //@todo implement for non-evenly-timed geometry!
        m_CreatedWorldGeometry->SetEvenlyTimed();
        slicedWorldGeometry->SetTimeBoundsInMS(worldTimeSlicedGeometry->GetGeometry3D(0)->GetTimeBoundsInMS());
      }
      m_CreatedWorldGeometry->SetGeometry3D(slicedWorldGeometry, 0);
    }
  }

  //unblock update; we may do this now, because if m_BlockUpdate was already true before this method was entered,
  //then we will never come here.
  m_BlockUpdate = false;

  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry and time/slice data.
  SendCreatedWorldGeometry();
  SendSlice();
  SendTime();
}

void mitk::SliceNavigationController::SendCreatedWorldGeometry()
{
  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry.
  if(!m_BlockUpdate)
    InvokeEvent(GeometrySendEvent(m_CreatedWorldGeometry, 0));
}

//##ModelId=3DD524D7038C
void mitk::SliceNavigationController::SendSlice()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));
      InvokeEvent(UpdateEvent());
    }
  }
}

void mitk::SliceNavigationController::SendTime()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()));
      InvokeEvent(UpdateEvent());
    }
  }
}

void mitk::SliceNavigationController::SetGeometry(const itk::EventObject & geometrySendEvent)
{
}

void mitk::SliceNavigationController::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
}

void mitk::SliceNavigationController::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
}

bool mitk::SliceNavigationController::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
    bool ok = false;
    //if (m_Destination == NULL)
    //    return false;
	
    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent!=NULL)
    {
      if(m_CreatedWorldGeometry.IsNull())
        return true;
      switch (sideEffectId)
      {
        case SeNEWPOINT:
       // {
			    //mitk::Point3D newPoint = posEvent->GetWorldPosition();
       //   vm2itk(newPoint, m_OldPoint);

			    //PointOperation* doOp = new mitk::PointOperation(OpADD, m_OldPoint, 0);
			    ////Undo
       //   if (m_UndoEnabled)
       //   {
				   // PointOperation* undoOp = new PointOperation(OpDELETE, m_OldPoint, 0);
       //     OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp,
						 //                                     					    objectEventId, groupEventId);
       //     m_UndoController->SetOperationEvent(operationEvent);
       //   }
       //   //execute the Operation
			    //m_Destination->ExecuteOperation(doOp);
       //   ok = true;
       //   break;
       // }
        case SeINITMOVEMENT:
       // {//move the point to the coordinate //not used, cause same to MovePoint... check xml-file
       //   mitk::ITKPoint3D movePoint;
       //   vm2itk(posEvent->GetWorldPosition(), movePoint);

       //   PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
       //   //execute the Operation
			    //m_Destination->ExecuteOperation(doOp);
       //   ok = true;
       //   break;
       // }
        case SeMOVEPOINT:
       // {
       //   mitk::ITKPoint3D movePoint;
       //   vm2itk(posEvent->GetWorldPosition(), movePoint);

       //   PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
       //   //execute the Operation
			    //m_Destination->ExecuteOperation(doOp);
       //   ok = true;
       //   break;
       // }
       // case SeFINISHMOVEMENT:
        {
       // {/*finishes a Movement from the coordinate supplier: 
       //   gets the lastpoint from the undolist and writes an undo-operation so 
       //   that the movement of the coordinatesupplier is undoable.*/
       //   mitk::ITKPoint3D movePoint, oldMovePoint;
       //   oldMovePoint.Fill(0);
       //   vm2itk(posEvent->GetWorldPosition(), movePoint);
       //   PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
       //   if (m_UndoEnabled )
       //   {
       //     //get the last Position from the UndoList
       //     OperationEvent *lastOperationEvent = m_UndoController->GetLastOfType(m_Destination, OpMOVE);
       //     if (lastOperationEvent != NULL)
       //     {
       //       PointOperation* lastOp = dynamic_cast<PointOperation *>(lastOperationEvent->GetOperation());
       //       if (lastOp != NULL)
       //       {
       //         oldMovePoint = lastOp->GetPoint();
       //       }
       //     }
       //     PointOperation* undoOp = new PointOperation(OpMOVE, oldMovePoint, 0);
       //     OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp,
						 //                                                   objectEventId, groupEventId);
       //     m_UndoController->SetOperationEvent(operationEvent);
       //   }
       //   //execute the Operation
	  		  //m_Destination->ExecuteOperation(doOp);


          mitk::Point3D point, projected_point; 
          point = posEvent->GetWorldPosition();
          
          //@todo add time to PositionEvent and use here!!
          SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
          if(slicedWorldGeometry!=NULL)
          {
            int best_slice = -1;
            double best_distance = itk::NumericTraits<double>::infinity();

            int s, slices;
            slices = slicedWorldGeometry->GetSlices();
            for(s=0; s < slices; ++s)
            {
              slicedWorldGeometry->GetGeometry2D(s)->Project(point, projected_point);
              Vector3D dist = projected_point-point;
              if(dist.lengthSquared() < best_distance)
              {
                best_distance = dist.lengthSquared();
                best_slice    = s;
              }
            }
            if(best_slice >= 0)
              GetSlice()->SetPos(best_slice);
          }
          ok = true;
          break;
        }
        default:
          ok = true;
          break;
        }
       return ok;
    }

    const mitk::DisplayPositionEvent* displPosEvent = dynamic_cast<const mitk::DisplayPositionEvent *>(stateEvent->GetEvent());
    if(displPosEvent!=NULL)
    {
        return true;
    }

	return false;
}
