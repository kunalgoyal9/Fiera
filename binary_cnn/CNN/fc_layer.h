#pragma once
#include <math.h>
#include <float.h>
#include <string.h>
#include "layer_t.h"
#include "optimization_method.h"
#include "gradient_t.h"

#pragma pack(push, 1)
struct fc_layer_t
{
	layer_type type = layer_type::fc;
	tensor_t<double> grads_in;
	tensor_t<double> in;
	tensor_t<double> out;
	std::vector<double> input;
	tensor_t<double> weights;
	tensor_t<gradient_t> gradients;
	bool debug,clip_gradients_flag;

	fc_layer_t( tdsize in_size, int out_size,bool clip_gradients_flag=true, bool debug_flag = false )
		:
		in( in_size.m, in_size.x, in_size.y, in_size.z ),
		out( in_size.m, out_size, 1, 1 ),
		grads_in( in_size.m, in_size.x, in_size.y, in_size.z ),
		weights( in_size.x*in_size.y*in_size.z, out_size, 1, 1 ),
		gradients(in_size.m, out_size, 1, 1)

	{
		this->debug = debug_flag;
		this->clip_gradients_flag = clip_gradients_flag;
		int maxval = in_size.x * in_size.y * in_size.z;
	
		// Weight initialization
		
		for ( int i = 0; i < out_size; i++ )
			for ( int h = 0; h < in_size.x*in_size.y*in_size.z; h++ )
				weights(h,i, 0, 0 ) =  (1.0f * (rand()-rand())) / double( RAND_MAX );  // Generates a random number between -1 and 1 
		
		if(debug)
		{
			cout << "********weights for fc************\n";
			print_tensor(weights);
		}
	}

	void activate( tensor_t<double>& in )
	{
		this->in = in;
		activate();
	}

	int map( point_t d )
	//  Maps weight unit to corresponding input unit.
	{
		return d.m * (in.size.x * in.size.y * in.size.z) +
			d.z * (in.size.x * in.size.y) +
			d.y * (in.size.x) +
			d.x;
	}

	void activate()
	/* 
	 * `activate` activates (forward propogate) the fc layer.
	 * It saves the result after propogation in `out` variable.
	 */
	{
		for ( int e = 0; e < in.size.m; e++)
			for ( int n = 0; n < out.size.x; n++ )
			{
				double inputv = 0;

				for ( int z = 0; z < in.size.z; z++ )
					for ( int j = 0; j < in.size.y; j++ )
						for ( int i = 0; i < in.size.x; i++ )
						{
							int m = map( { 0 , i, j, z } );
							inputv += in( e, i, j, z ) * weights(m, n, 0, 0 );
						}

				out( e, n, 0, 0 ) = inputv;
		}
		
		if(debug)
		{
			cout<<"*******output for fc**********\n";
			print_tensor(out);
		}
	}

	void fix_weights(double learning_rate)
	{
		for ( int n = 0; n < out.size.x; n++ )
		{
			for ( int i = 0; i < in.size.x; i++ )
				for ( int j = 0; j < in.size.y; j++ )
					for ( int z = 0; z < in.size.z; z++ )
					{
						int m = map( { 0, i, j, z } );

						double& w = weights( m, n, 0, 0 );
						
						gradient_t grad_sum;
						gradient_t weight_grad;
						
						for ( int e = 0; e < out.size.m; e++ ){			
							weight_grad = gradients(e, n, 0, 0) * in(e, i, j, z);	// d W = d A(l+1) * A(l)
							grad_sum = weight_grad + grad_sum;
						}
						
						// grad_sum = grad_sum / out.size.m;
						w = update_weight( w, grad_sum, 1, false, learning_rate); 
					}
			for (int e = 0; e < out.size.m; e++)
				update_gradient( gradients(e, n, 0, 0) );
		}

		if(debug)
		{
			cout<<"*******new weights for double fc*****\n";
			print_tensor(weights);
		}
	}

	void calc_grads( tensor_t<double>& grad_next_layer )
	
	// Calculates backward propogation and saves result in `grads_in`. 
	{
		memset( grads_in.data, 0, grads_in.size.x *grads_in.size.y*grads_in.size.z * grads_in.size.m * sizeof( double ) );
		
		for(int e=0; e<in.size.m; e++)
			for ( int n = 0; n < out.size.x; n++ )
			{
				gradient_t& grad = gradients(e,n,0,0);
				grad.grad = grad_next_layer(e, n, 0, 0 );

				for ( int i = 0; i < in.size.x; i++ )
					for ( int j = 0; j < in.size.y; j++ )
						for ( int z = 0; z < in.size.z; z++ )
						{
							int m = map( {0, i, j, z } );
							grads_in(e, i, j, z ) += grad.grad * weights( m, n,0, 0 );
							clip_gradients(clip_gradients_flag, grads_in(e,i,j,z));
						}
			}
		
		if(debug)
		{
			cout<<"**********grads_in for double fc***********\n";
			print_tensor(grads_in);
		}
		

	}
};
#pragma pack(pop)
