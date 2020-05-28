#pragma once
#include <Utils\FPoint.h>

namespace bla {
	template <typename T>
	class Vector2 {
		static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Type must be integral or floating point");
	public:
		Vector2() = delete;
		
		Vector2(FPoint first, FPoint second) {
			x = second.x - first.x;
			y = second.y - first.y;

		};
		
		Vector2(T _X, T _Y) {
			x = _X;
			y = _Y;
		}
		
		~Vector2() {}

		float Dot(Vector2<T>& other) {
			return x * other.x + y * other.y;
		}

		Vector2<T> Dot(float scalar) {
			return Vector2<T>(x * scalar, y * scalar);
		}

		Vector2<T> Normal() {
			return Vector2<T>(-y, x);
		}

		float Angle(Vector2<T>& other) {
			return acos(Dot(other) / (Length() * other.Length()));
		}

		Vector2<T> Projection(Vector2<T>& onto) {
			auto num = Dot(onto);
			auto denom = onto.Dot(onto);
			if (denom == 0.0) {
				denom = 1;
			}
			return onto.Dot(num/denom);
		}
	
		float Length() {
			return sqrt(Dot(Vector2<float>(x, y)));
		}

		float x;
		float y;
	};

}

template <typename T>
bla::Vector2<T> operator+ (bla::Vector2<T>& lhs, bla::Vector2<T>& rhs) {
	return bla::Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}

template <typename T>
bla::Vector2<T> operator- (bla::Vector2<T>& lhs, bla::Vector2<T>& rhs) {
	return bla::Vector2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}
template <typename T>
bla::Vector2<T> operator- (bla::Vector2<T>& vec) {
	return bla::Vector2<T>(-vec.x, -vec.y);
}