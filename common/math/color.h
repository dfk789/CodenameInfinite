#ifndef LW_COLOR_H
#define LW_COLOR_H

#include "maths.h"

template <class unit_t>
class TemplateVector;

class Color
{
public:
	inline			Color();
	inline			Color(class TemplateVector<float> v);
	inline			Color(int _r, int _g, int _b);
	inline			Color(int _r, int _g, int _b, int _a);
	inline			Color(float _r, float _g, float _b);
	inline			Color(float _r, float _g, float _b, float _a);

	inline void		SetColor(int _r, int _g, int _b, int _a);
	inline void		SetColor(float _r, float _g, float _b, float _a);
	inline void		SetRed(int _r);
	inline void		SetGreen(int _g);
	inline void		SetBlue(int _b);
	inline void		SetAlpha(int _a);
	inline void		SetAlpha(float f);

	int				r() const { return red; };
	int				g() const { return green; };
	int				b() const { return blue; };
	int				a() const { return alpha; };

	Color	operator-(void) const;

	Color	operator+(const Color& v) const;
	Color	operator-(const Color& v) const;
	Color	operator*(float s) const;
	Color	operator/(float s) const;

	void	operator+=(const Color &v);
	void	operator-=(const Color &v);
	void	operator*=(float s);
	void	operator/=(float s);

	Color	operator*(const Color& v) const;

	friend Color operator*( float f, const Color& v )
	{
		return Color( v.red*f, v.green*f, v.blue*f, v.alpha*f );
	}

	friend Color operator/( float f, const Color& v )
	{
		return Color( f/v.red, f/v.green, f/v.blue, f/v.alpha );
	}

	operator unsigned char*()
	{
		return(&red);
	}

	operator const unsigned char*() const
	{
		return(&red);
	}

private:
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	unsigned char	alpha;
};

#include "vector.h"

Color::Color()
{
	Color(0, 0, 0, 255);
}

Color::Color(Vector v)
{
	SetColor((int)(v.x*255), (int)(v.y*255), (int)(v.z*255), 255);
}

Color::Color(int _r, int _g, int _b)
{
	SetColor(_r, _g, _b, 255);
}

Color::Color(int _r, int _g, int _b, int _a)
{
	SetColor(_r, _g, _b, _a);
}

Color::Color(float _r, float _g, float _b)
{
	SetColor(_r, _g, _b, 1.0f);
}

Color::Color(float _r, float _g, float _b, float _a)
{
	SetColor(_r, _g, _b, _a);
}

void Color::SetColor(int _r, int _g, int _b, int _a)
{
	red = _r;
	green = _g;
	blue = _b;
	alpha = _a;
}

void Color::SetColor(float _r, float _g, float _b, float _a)
{
	red = (int)(_r*255);
	green = (int)(_g*255);
	blue = (int)(_b*255);
	alpha = (int)(_a*255);
}

void Color::SetRed(int _r)
{
	red = _r;
}

void Color::SetGreen(int _g)
{
	green = _g;
}

void Color::SetBlue(int _b)
{
	blue = _b;
}

void Color::SetAlpha(int _a)
{
	alpha = _a;
}

void Color::SetAlpha(float f)
{
	alpha = (int)(f*255);
}

inline Color Color::operator-() const
{
	return Color(255-red, 255-green, 255-blue, alpha);
}

inline Color Color::operator+(const Color& v) const
{
	return Color(red+v.red, green+v.green, blue+v.blue, alpha+v.alpha);
}

inline Color Color::operator-(const Color& v) const
{
	return Color(red-v.red, green-v.green, blue-v.blue, alpha-v.alpha);
}

inline Color Color::operator*(float s) const
{
	return Color((int)(red*s), (int)(green*s), (int)(blue*s), (int)alpha);
}

inline Color Color::operator/(float s) const
{
	return Color((int)(red/s), (int)(green/s), (int)(blue/s), (int)alpha);
}

inline void Color::operator+=(const Color& v)
{
	red += v.red;
	green += v.green;
	blue += v.blue;
}

inline void Color::operator-=(const Color& v)
{
	red -= v.red;
	green -= v.green;
	blue -= v.blue;
}

inline void Color::operator*=(float s)
{
	red = (int)(s*red);
	green = (int)(s*green);
	blue = (int)(s*blue);
}

inline void Color::operator/=(float s)
{
	red = (int)(red/s);
	green = (int)(green/s);
	blue = (int)(blue/s);
}

inline Color Color::operator*(const Color& v) const
{
	return Color(red*(float)(v.red/255), green*(float)(v.green/255), blue*(float)(v.blue/255), alpha*(float)(v.alpha/255));
}

#endif
