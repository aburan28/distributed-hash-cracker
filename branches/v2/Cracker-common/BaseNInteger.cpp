/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v2.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
*******************************************************************************
*                                                                             *
* BaseNInteger.cpp - arbitrary-base integer class                             *
*                                                                             *
******************************************************************************/

#include <string>
#include <vector>
#include <cassert>
#include "BaseNInteger.h"

using namespace std;

BaseNInteger::BaseNInteger(int b)
{
	base=b;
	
	SetLength(1);
}

void BaseNInteger::SetLength(int len)
{
	if(len > 50)
		throw string("BaseNInteger does not support lengths of over 50");
	size = len;

	//Clear digits and place value chart
	for(int i=0;i<len;i++)
	{
		digits[i]=0;
		placevalue[i]=1;
	}

	//Update place value chart
	for(int i=len-2;i>=0;i--)
		placevalue[i] = base * placevalue[i+1];
}

bool BaseNInteger::IncrementWithSaturation()
{
	//Bump and check for carry
	int s = size-1;
	if( ++digits[s] == base)
	{
		//Handle the carry. If we overflowed our most significant digit, return "overflow"
		while(s>=0)
		{
			//Did we overflow our most significant digit?
			//If we did, saturate and quit.
			if(s==0)
			{
				for(int k=0; k<size; k++)
					digits[k] = base-1;
				return true;
			}

			//Current digit overflowed. Zero us while bumping the digit one to our left.
			digits[s] = 0;
			if( ++digits[ --s ] != base)	//Did we absorb the carry? If so, we're done.
				break;
		}
	}
	return false;
}

bool BaseNInteger::AddWithSaturation(long long n)
{
	//Modified "making change" algorithm to make sure we don't overflow low-order digits
	for(int i=0;i<size;i++)
	{
		long long pv=placevalue[i];

		//Add to the current digit, handling overflow if it occurs
		int delta = static_cast<int>(n/pv);
		digits[i] += delta;
		n-=pv*delta;
		
		//Check for carry
		if(digits[i]>=base)
		{
			//Handle the carry. If we overflowed our most significant digit, return "overflow"
			for(int j=i;j>=0;j--)
			{
				//Did we overflow our most significant digit?
				//If we did, saturate and quit.
				if(j==0)
				{
					for(int k=0;k<size;k++)
						digits[k]=base-1;
					return true;
				}

				//Current digit overflowed. Subtract the base while bumping the digit one to our left.
				digits[j]-=base;
				digits[j-1]++;

				//Did we absorb the carry? If so, we're done.
				if(digits[j-1]<base)
					break;
			}

		}
	}

	return false;
}

bool BaseNInteger::operator<(const BaseNInteger& rhs)
{
	//should never happen in the cracker - all elements in a WU are the same size and base
	assert(base == rhs.base);
	assert(size == rhs.size);

	//Check from L-R
	for(int i=0; i<size; i++)
	{
		int d = digits[i] - rhs.digits[i];
		if(d < 0)
			return true;
		if(d > 0)
			return false;
	}
	return false;
}

bool BaseNInteger::operator==(const BaseNInteger& rhs)
{
	//Incomparable? If so, not equal
	if(base!=rhs.base)
		return false;
	if(size!=rhs.size)
		return false;

	//No - check digits
	for(int i=0;i<size;i++)
	{
		if(digits[i]!=rhs.digits[i])
			return false;
	}

	return true;
}


bool BaseNInteger::operator!=(const BaseNInteger& rhs)
{
	return !(*this == rhs);
}

long long BaseNInteger::toInt()
{
	long long ret=0;
	for(int i=0;i<size;i++)
		ret+=digits[i] * placevalue[i];
	return ret;
}
