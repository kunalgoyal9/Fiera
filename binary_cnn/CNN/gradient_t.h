#pragma once


struct gradient_t
{
	double grad;
	double oldgrad;
	gradient_t()
	{
		grad = 0;
		oldgrad = 0;
	}

	gradient_t operator+( gradient_t& other )
	{
		gradient_t clone( *this );
		clone.grad += other.grad;
		clone.oldgrad += other.oldgrad;
		return clone;
	}

	gradient_t operator/( double num )
	{
		gradient_t clone( *this );
		clone.grad /= num;
		clone.oldgrad /= num;
		return clone;
	}	

	gradient_t operator*( double num )
	{
		gradient_t clone( *this );
		clone.grad *= num;
		clone.oldgrad *= num;
		return clone;
	}
};