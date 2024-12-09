#include"Camera.h"

SDL_Rect Camera::GetViewBox()
{
	return m_ViewBox;
}

Vector2 Camera::GetPosition()
{
	return m_Position;
}

void Camera::SetTarget(std::shared_ptr<BaseObject> target) // Settarget for player (-> Set origin position)
{
	m_target = target;
}

void Camera::Update(float deltaTime)
{
	if (m_target != nullptr)
	{
		//Center the camera over the dot
		m_ViewBox.x = (m_target->GetPosition().x - SCREEN_WIDTH / 2);
		m_ViewBox.y = (m_target->GetPosition().y  - SCREEN_HEIGHT / 2) ;
		
		//Keep the camera in bounds
		if (m_ViewBox.x < 0)
		{
			m_ViewBox.x = 0;
		}
		if (m_ViewBox.y < 0)
		{
			m_ViewBox.y = 0;
		}
		if (m_ViewBox.x > (GetLevelWidth() - m_ViewBox.w))
		{
			m_ViewBox.x = (GetLevelWidth() - m_ViewBox.w);
		}
		if (m_ViewBox.y > (GetLevelHeight() - m_ViewBox.h))
		{
			m_ViewBox.y = (GetLevelHeight() - m_ViewBox.h);
		}
		m_Position = Vector2(m_ViewBox.x, m_ViewBox.y);
	}
}

void Camera::SetLevelDimension(int width, int height)
{
	m_levelWidth = width;
	m_levelHeight = height;
}

int Camera::GetLevelWidth()
{
	return m_levelWidth;
}

int Camera::GetLevelHeight()
{
	return m_levelHeight;
}
