/**
 * @file thlayoutclr.cxx
 */
  
/* Copyright (C) 2000 Stacho Mudrak
 * 
 * $Date: $
 * $RCSfile: $
 * $Revision: $
 *
 * -------------------------------------------------------------------- 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * --------------------------------------------------------------------
 */
 
#include "thlayoutclr.h"
#include "thparse.h"
#include "thdatabase.h"
#include "thexception.h"
#include <cmath>


bool thlayout_color::is_defined() {
  return (this->defined > 0);
}


void thlayout_color::set_color(int output_model, color & clr) {
	this->fill_missing_color_models();
	if ((output_model & TT_LAYOUTCLRMODEL_CMYK) > 0)
		clr.set(this->C, this->M, this->Y, this->K);
	else if ((output_model & TT_LAYOUTCLRMODEL_GRAY) > 0)
		clr.set(this->W);
	else
		clr.set(this->R, this->G, this->B);
}


void thlayout_color::fill_missing_color_models() {
	if ((this->model & TT_LAYOUTCLRMODEL_RGB) > 0) {
		if ((this->model & TT_LAYOUTCLRMODEL_CMYK) == 0)
			this->RGBtoCMYK();
		if ((this->model & TT_LAYOUTCLRMODEL_GRAY) == 0)
			this->RGBtoGRAYSCALE();
	}
	if ((this->model & TT_LAYOUTCLRMODEL_CMYK) > 0) {
		if ((this->model & TT_LAYOUTCLRMODEL_RGB) == 0)
			this->CMYKtoRGB();
		if ((this->model & TT_LAYOUTCLRMODEL_GRAY) == 0)
			this->RGBtoGRAYSCALE();
	}
	if ((this->model & TT_LAYOUTCLRMODEL_GRAY) > 0) {
		if ((this->model & TT_LAYOUTCLRMODEL_RGB) == 0)
			this->GRAYSCALEtoRGB();
		if ((this->model & TT_LAYOUTCLRMODEL_CMYK) == 0)
			this->RGBtoCMYK();
	}
}


void thlayout_color::print_to_file(int output_model, FILE * f) {
	this->fill_missing_color_models();
	if ((output_model & TT_LAYOUTCLRMODEL_CMYK) > 0)
		fprintf(f, "(%.5f,%.5f,%.5f,%.5f)", this->C, this->M, this->Y, this->K);
	else if ((output_model & TT_LAYOUTCLRMODEL_GRAY) > 0)
		fprintf(f, "(%.5f)", this->W);
	else
		fprintf(f, "(%.5f,%.5f,%.5f)", this->R, this->G, this->B);
}


void thlayout_color::RGBtoGRAYSCALE() {
	this->W = 0.2126 * this->R + 0.7152 * this->G + 0.0722 * this->B;
	if (this->W <= 0.0031308) {
		this->W *= 12.92;
	} else {
		this->W = 1.055 * pow(this->W, 1/2.4) - 0.055;
	}
}

void thlayout_color::GRAYSCALEtoRGB() {
	this->R = this->G = this->B = this->W;
}


void thlayout_color::RGBtoCMYK() {
    this->C = 1.0 - this->R;
    this->M = 1.0 - this->G;
    this->Y = 1.0 - this->B;
    this->K = this->C;
    if (this->K > this->M) this->K = this->M;
    if (this->K > this->Y) this->K = this->Y;
    if (this->K == 1.0) {
    	this->C = 0.0;
    	this->M = 0.0;
    	this->Y = 0.0;
    } else {
    	this->C = (1.0 - this->R - this->K) / (1.0 - this->K);
    	this->M = (1.0 - this->G - this->K) / (1.0 - this->K);
    	this->Y = (1.0 - this->B - this->K) / (1.0 - this->K);
    }
}

void thlayout_color::CMYKtoRGB() {
  	this->R = (1.0 - this->C) * (1.0 - this->K);
  	this->G = (1.0 - this->M) * (1.0 - this->K);
  	this->B = (1.0 - this->Y) * (1.0 - this->K);
}

void thlayout_color::parse(char * str, bool aalpha) {
	// 1 arg = W (grayscale)
	// 3 arg = RGB
	// 4 arg = CMYK
	// 8 arg = RGBCMYKW
  thsplit_words(&(thdb.mbuff_tmp), str);
  int nargs = thdb.mbuff_tmp.get_size(), sv;
  char ** args = thdb.mbuff_tmp.get_buffer();
#define invalid_color_spec ththrow("invalid color specification -- {}", str);
  switch (nargs) {
    case 8:
			thparse_double(sv,this->C,args[4]);
			if ((sv != TT_SV_NUMBER) || (this->C < 0.0) || (this->C > 100.0))
				invalid_color_spec;
			thparse_double(sv,this->M,args[5]);
			if ((sv != TT_SV_NUMBER) || (this->M < 0.0) || (this->M > 100.0))
				invalid_color_spec;
			thparse_double(sv,this->Y,args[6]);
			if ((sv != TT_SV_NUMBER) || (this->Y < 0.0) || (this->Y > 100.0))
				invalid_color_spec;
			thparse_double(sv,this->K,args[7]);
			if ((sv != TT_SV_NUMBER) || (this->K < 0.0) || (this->K > 100.0))
				invalid_color_spec;
			this->C /= 100.0;
			this->M /= 100.0;
			this->Y /= 100.0;
			this->K /= 100.0;
  	case 4:
			thparse_double(sv,this->W,args[3]);
			if ((sv != TT_SV_NUMBER) || (this->W < 0.0) || (this->W > 100.0))
				invalid_color_spec;
			this->W /= 100.0;
    case 3:
      thparse_double(sv,this->B,args[2]);        
      if ((sv != TT_SV_NUMBER) || (this->B < 0.0) || (this->B > 100.0))
        invalid_color_spec;
      this->B /= 100.0;
      thparse_double(sv,this->G,args[1]);        
      if ((sv != TT_SV_NUMBER) || (this->G < 0.0) || (this->G > 100.0))
        invalid_color_spec;
      this->G /= 100.0;
    case 1:
      if (aalpha && (strcmp(args[0],"transparent") == 0)) {
        this->A = 0.0;
        this->defined = 2;
        break;
      }
      thparse_double(sv,this->R,args[0]);        
      if ((sv != TT_SV_NUMBER) || (this->R < 0.0) || (this->R > 100.0))
        invalid_color_spec;
      this->R /= 100.0;
      if (nargs == 1) {
        this->B = this->R;
        this->G = this->R;
        this->W = this->R;
        this->K = 1.0 - this->R;
        this->C = 0.0;
        this->M = 0.0;
        this->Y = 0.0;
        this->model = TT_LAYOUTCLRMODEL_GRAY;
      }
      if (nargs == 3) {
        this->RGBtoCMYK();
        this->RGBtoGRAYSCALE();
        this->model = TT_LAYOUTCLRMODEL_RGB;
      }
      if (nargs == 4) {
      	this->C = this->R;
      	this->M = this->G;
      	this->Y = this->B;
      	this->K = this->W;
      	this->CMYKtoRGB();
      	this->RGBtoGRAYSCALE();
        this->model = TT_LAYOUTCLRMODEL_CMYK;
      }
      if (nargs == 8) {
      	double tmp;
      	tmp = this->K;
      	this->K = this->Y;
      	this->Y = this->M;
      	this->M = this->C;
      	this->C = this->W;
      	this->W = tmp;
      	this->model = TT_LAYOUTCLRMODEL_CMYK | TT_LAYOUTCLRMODEL_GRAY | TT_LAYOUTCLRMODEL_RGB;
      }
      this->defined = 2;
      break;
    default:
      invalid_color_spec;
  }
}

void thlayout_color::alpha_correct(double alpha) {
	this->R = alpha * this->R + (1 - alpha) * 1.0;
	this->G = alpha * this->G + (1 - alpha) * 1.0;
	this->B = alpha * this->B + (1 - alpha) * 1.0;
	this->W = alpha * this->W + (1 - alpha) * 1.0;
	this->C = alpha * this->C + (1 - alpha) * 0.0;
	this->M = alpha * this->M + (1 - alpha) * 0.0;
	this->Y = alpha * this->Y + (1 - alpha) * 0.0;
	this->K = alpha * this->K + (1 - alpha) * 0.0;
}

/**
 * Mix color with ratio * other_clr.
 */

void thlayout_color::mix_with_color(double ratio, thlayout_color other_clr) {
	this->R = (1 - ratio) * this->R + ratio * other_clr.R;
	this->G = (1 - ratio) * this->G + ratio * other_clr.G;
	this->B = (1 - ratio) * this->B + ratio * other_clr.B;
	this->W = (1 - ratio) * this->W + ratio * other_clr.W;
	this->C = (1 - ratio) * this->C + ratio * other_clr.C;
	this->M = (1 - ratio) * this->M + ratio * other_clr.M;
	this->Y = (1 - ratio) * this->Y + ratio * other_clr.Y;
	this->K = (1 - ratio) * this->K + ratio * other_clr.K;
}
