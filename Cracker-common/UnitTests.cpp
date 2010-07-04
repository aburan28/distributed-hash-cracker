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
* UnitTests.cpp - unit tests for this module                                  *
*                                                                             *
******************************************************************************/
#include "config.h"

#include "Cracker-common.h"
#include "BaseNInteger.h"

#include <iostream>
using namespace std;

extern "C" CRACKER_COMMON_EXPORT void UnitTests();

void BaseNIntegerTest();

void UnitTests()
{
	cout << "\033[1;34mBeginning unit tests for module Cracker-common\033[0m" << endl;
	
	BaseNIntegerTest();
	
	cout << endl;
}

void BaseNIntegerTest()
{
	cout << " * BaseNInteger... ";
	
	try
	{
		//Create a 3-digit base 10 integer
		BaseNInteger num(10);
		num.SetLength(3);

		//Run some tests
		num.IncrementWithSaturation();
		if(num.digits[0] != 0 || num.digits[1] != 0 || num.digits[2] != 1)
			throw 1;
		
		num.AddWithSaturation(9);
		if(num.digits[0] != 0 || num.digits[1] != 1 || num.digits[2] != 0)
			throw 2;
			
		num.AddWithSaturation(74);
		if(num.digits[0] != 0 || num.digits[1] != 8 || num.digits[2] != 4)
			throw 3;
		
		num.IncrementWithSaturation();
		if(num.digits[0] != 0 || num.digits[1] != 8 || num.digits[2] != 5)
			throw 4;
			
		num.AddWithSaturation(20);
		if(num.digits[0] != 1 || num.digits[1] != 0 || num.digits[2] != 5)
			throw 5;
			
		cout << "\033[32mpassed\033[0m" << endl;
	}
	catch(int i)
	{
		cout << "\033[1;31mFAILED (test #" << i << ")\033[0m" << endl;
	}
}
