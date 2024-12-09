#include "MouseButton.h"

MouseButton::MouseButton(std::shared_ptr<TextureManager> texture, SDL_RendererFlip flip)
	: Sprite2D(texture,flip), m_pBtClick(nullptr)
{

}

bool MouseButton::HandleTouchEvent(SDL_Event* e)
{
	//If mouse event happened (clicked/released...
	if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
	{
		//Get mouse position
		int x, y;
		SDL_GetMouseState(&x, &y);
		// Check if mouse is in button
		bool inside = true;
		//Mouse is left/right above/below of the button
		if (x < m_position.x || x > m_position.x + m_iWidth|| y < m_position.y || y > m_position.y + m_iHeight)
		{
			inside = false;
		}
		// Mouse is outside button
		if(inside)
		{
			// Only perform click action when the same button was pressed down and released
			m_pBtClick();
		}
		//if (e->button.button == SDL_BUTTON_LEFT)
		//{
			///  perform left click action when the same button was pressed down and released with m_pBtLeftClick() callback.
		//}
		return inside;
	}
	return false;
}

void MouseButton::SetOnClick(std::function<void(void)> pBtClick)
{
	m_pBtClick = pBtClick;
}





