#include "GameObject3D.h"

void GameObject3D::SetLookAtPos(XMFLOAT3 lookAtPos)
{
	// Verify that llok at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
	if (lookAtPos.x == this->pos.x && lookAtPos.y == this->pos.y && lookAtPos.z == this->pos.z)
	{
		return;
	}

	lookAtPos.x = this->pos.x - lookAtPos.x;
	lookAtPos.y = this->pos.y - lookAtPos.y;
	lookAtPos.z = this->pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
	{
		yaw += XM_PI;
	}

	this->SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR& GameObject3D::GetForwardVector(bool omitY)
{
	if (omitY)
	{
		return this->vec_backward_noY;
	}
	else
	{
		return this->vec_forward;
	}
}

const XMVECTOR& GameObject3D::GetRightVector(bool omitY)
{
	if (omitY)
	{
		return this->vec_right_noY;
	}
	else
	{
		return this->vec_right;
	}
}

const XMVECTOR& GameObject3D::GetBackwardVector(bool omitY)
{
	if (omitY)
	{
		return this->vec_backward_noY;
	}
	else
	{
		return this->vec_backward;
	}
}

const XMVECTOR& GameObject3D::GetLeftVector(bool omitY)
{
	if (omitY)
	{
		return this->vec_left_noY;
	}
	else
	{
		return this->vec_left;
	}
}

void GameObject3D::UpdateMatrix()
{
	assert("UpdateMatrix must be overridden." && 0);
}

void GameObject3D::UpdateDirectionVectors()
{
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, 0.0f);
	this->vec_forward = XMVector3TransformCoord(this->DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	this->vec_backward = XMVector3TransformCoord(this->DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	this->vec_left = XMVector3TransformCoord(this->DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	this->vec_right = XMVector3TransformCoord(this->DEFAULT_RIGHT_VECTOR, vecRotationMatrix);

	XMMATRIX vecRotationMatrixNoY = XMMatrixRotationRollPitchYaw(0.0f, this->rot.y, 0.0f);
	this->vec_forward_noY = XMVector3TransformCoord(this->DEFAULT_FORWARD_VECTOR, vecRotationMatrixNoY);
	this->vec_backward_noY = XMVector3TransformCoord(this->DEFAULT_BACKWARD_VECTOR, vecRotationMatrixNoY);
	this->vec_left_noY = XMVector3TransformCoord(this->DEFAULT_LEFT_VECTOR, vecRotationMatrixNoY);
	this->vec_right_noY = XMVector3TransformCoord(this->DEFAULT_RIGHT_VECTOR, vecRotationMatrixNoY);
}
