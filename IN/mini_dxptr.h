#pragma once
#pragma warning( push )
#pragma warning( disable : 4005)
#include <D3D11.h>
#pragma warning( pop ) 
#include <memory>

namespace mini
{
	template<class T>
	class DxDeleter
	{
	public:
		typedef T* pointer;

		void operator () (T* comObject) { if (comObject) comObject->Release(); }
	};

	template<class T>
	class dx_ptr : public std::unique_ptr<T, DxDeleter<T>>
	{
	public:
		dx_ptr() : unique_ptr(nullptr) { }
		dx_ptr(nullptr_t p) : unique_ptr(p) { }
		dx_ptr(T* p) : unique_ptr(p) { }
		dx_ptr(dx_ptr&& right) : unique_ptr(std::move(right)) { }

		dx_ptr& operator=(dx_ptr&& right)
		{ std::unique_ptr<T, DxDeleter<T>>::operator=(std::move(right)); return *this; }

	private:
		dx_ptr(const dx_ptr& right) { /*do not use*/ }
		dx_ptr& operator=(const dx_ptr& right) { /*do not use*/ }
	};
}