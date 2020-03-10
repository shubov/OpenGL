#include "common_header.h"

#include "win_OpenGLApp.h"
#include "flyingCamera.h"

#include <glm/gtx/rotate_vector.hpp>

const float PI = float(atan(1.0)*4.0);

CFlyingCamera::CFlyingCamera()
{
	vEye = glm::vec3(0.0f, 0.0f, 0.0f);
	vView = glm::vec3(0.0f, 0.0, -1.0f);
	vUp = glm::vec3(0.0f, 1.0f, 0.0f);
	fSpeed = 25.0f;
	fSensitivity = 0.1f;
}

CFlyingCamera::CFlyingCamera(glm::vec3 a_vEye, glm::vec3 a_vView, glm::vec3 a_vUp, float a_fSpeed, float a_fSensitivity)
{
	vEye = a_vEye; vView = a_vView; vUp = a_vUp;
	fSpeed = a_fSpeed;
	fSensitivity = a_fSensitivity;
}

/*-----------------------------------------------
Name:	rotateWithMouse
Params:	none
Result:	Checks for moving of mouse and rotates
		camera.
/*---------------------------------------------*/

void CFlyingCamera::RotateWithMouse()
{
	GetCursorPos(&pCur);
	RECT rRect; GetWindowRect(appMain.hWnd, &rRect);
	int iCentX = (rRect.left+rRect.right)>>1,
		iCentY = (rRect.top+rRect.bottom)>>1;

	float deltaX = (float)(iCentX-pCur.x)*fSensitivity;
	float deltaY = (float)(iCentY-pCur.y)*fSensitivity;

	if(deltaX != 0.0f)
	{
		vView -= vEye;
		vView = glm::rotate(vView, deltaX, glm::vec3(0.0f, 1.0f, 0.0f));
		vView += vEye;
	}
	if(deltaY != 0.0f)
	{
		glm::vec3 vAxis = glm::cross(vView-vEye, vUp);
		vAxis = glm::normalize(vAxis);
		float fAngle = deltaY;
		float fNewAngle = fAngle+GetAngleX();
		if(fNewAngle > -89.80f && fNewAngle < 89.80f)
		{
			vView -= vEye;
			vView = glm::rotate(vView, deltaY, vAxis);
			vView += vEye;
		}
	}
	SetCursorPos(iCentX, iCentY);
}

/*-----------------------------------------------
Name:	GetAngleY
Params:	none
Result:	Gets Y angle of camera (head turning left
		and right).
/*---------------------------------------------*/

float CFlyingCamera::GetAngleY()
{
	glm::vec3 vDir = vView-vEye; vDir.y = 0.0f;
	glm::normalize(vDir);
	float fAngle = acos(glm::dot(glm::vec3(0, 0, -1), vDir))*(180.0f/PI);
	if(vDir.x < 0)fAngle = 360.0f-fAngle;
	return fAngle;
}

/*-----------------------------------------------
Name:	GetAngleX
Params:	none
Result:	Gets X angle of camera (head turning up
		and down).
/*---------------------------------------------*/

float CFlyingCamera::GetAngleX()
{
	glm::vec3 vDir = vView-vEye;
	vDir = glm::normalize(vDir);
	glm::vec3 vDir2 = vDir; vDir2.y = 0.0f;
	vDir2 = glm::normalize(vDir2);
	float fAngle =  acos(glm::dot(vDir2, vDir))*(180.0f/PI);
	if(vDir.y < 0)fAngle *= -1.0f;
	return fAngle;
}

/*-----------------------------------------------
Name:	SetMovingKeys
Params:	a_iForw - move forward Key
		a_iBack - move backward Key
		a_iLeft - strafe left Key
		a_iRight - strafe right Key
Result:	Sets Keys for moving camera.
/*---------------------------------------------*/

void CFlyingCamera::SetMovingKeys(int a_iForw, int a_iBack, int a_iLeft, int a_iRight)
{
	iForw = a_iForw;
	iBack = a_iBack;
	iLeft = a_iLeft;
	iRight = a_iRight;
}

/*-----------------------------------------------
Name:	Update
Params:	none
Result:	Performs updates of camera - moving and
		rotating.
/*---------------------------------------------*/

void CFlyingCamera::Update()
{
	RotateWithMouse();

	// Get view direction
	glm::vec3 vMove = vView-vEye;
	vMove = glm::normalize(vMove);
	vMove *= fSpeed;

	glm::vec3 vStrafe = glm::cross(vView-vEye, vUp);
	vStrafe = glm::normalize(vStrafe);
	vStrafe *= fSpeed;

	int iMove = 0;
	glm::vec3 vMoveBy;
	// Get vector of move
	if(Keys::Key(iForw))vMoveBy += vMove*appMain.sof(1.0f);
	if(Keys::Key(iBack))vMoveBy -= vMove*appMain.sof(1.0f);
	if(Keys::Key(iLeft))vMoveBy -= vStrafe*appMain.sof(1.0f);
	if(Keys::Key(iRight))vMoveBy += vStrafe*appMain.sof(1.0f);
	vEye += vMoveBy; vView += vMoveBy;
}

/*-----------------------------------------------
Name:	ResetMouse
Params:	none
Result:	Sets mouse cursor back to the center of
		window.
/*---------------------------------------------*/

void CFlyingCamera::ResetMouse()
{
	RECT rRect; GetWindowRect(appMain.hWnd, &rRect);
	int iCentX = (rRect.left+rRect.right)>>1,
		iCentY = (rRect.top+rRect.bottom)>>1;
	SetCursorPos(iCentX, iCentY);
}

/*-----------------------------------------------
Name:	Look
Params:	none
Result:	Returns proper modelview matrix to make
		camera look.
/*---------------------------------------------*/

glm::mat4 CFlyingCamera::Look()
{
	return glm::lookAt(vEye, vView, vUp);
}