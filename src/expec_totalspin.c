/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 Takahiro Misawa, Kazuyoshi Yoshimi, Mitsuaki Kawamura, Youhei Yamaji, Synge Todo, Naoki Kawashima */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#include <bitcalc.h>
#include "expec_totalspin.h"

/**
 * @file   expec_totalspin.c
 * 
 * @brief  File for calculating total spin
 * 
 * @version 0.2
 * @details modify to treat the case of general spin 
 *
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 */


/** 
 * @brief Parent function of calculation of total spin
 * 
 * @param[in,out] X data list of calculation parameters
 * @param[in] vec eigenvectors
 * 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * @retval 0 calculation is normally finished
 */
int expec_totalspin
(
 struct BindStruct *X,
 double complex *vec
 )
{
  switch(X->Def.iCalcModel){
  case Spin:
     totalspin_Spin(X,vec);
     X->Phys.sz=X->Def.Total2Sz/2.;
     break;
  case SpinGC:
     totalspin_SpinGC(X,vec);
     break;
   case Hubbard: 
   case Kondo:
     totalspin_Hubbard(X,vec);
     X->Phys.sz=X->Def.Total2Sz/2.;
     break;
  case HubbardGC: 
  case KondoGC:
     totalspin_HubbardGC(X,vec);
     break;
  default:
    X->Phys.s2=0.0;   
    X->Phys.sz=0.0;   
  }
  return 0;
}

/** 
 * @brief function of calculating totalspin for Hubbard model
 * 
 * @param[in,out] X data list of calculation parameters
 * @param vec eigenvectors
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
void totalspin_Hubbard(struct BindStruct *X,double complex *vec){ 
  long unsigned int j;
  long unsigned int irght,ilft,ihfbit;
  long unsigned int isite1,isite2;
  long unsigned int is1_up,is2_up,is1_down,is2_down;
  long unsigned int iexchg, off;
  int num1_up,num2_up;
  int num1_down,num2_down; 
  long unsigned int ibit1_up,ibit2_up,ibit1_down,ibit2_down; 
  double complex spn_z,tmp_spn_z;
  double complex spn;
  long unsigned i_max;    
  i_max=X->Check.idim_max;

  GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, &irght, &ilft, &ihfbit);
  spn   = 0.0;
  spn_z = 0.0;
  for(isite1=1;isite1<=X->Def.Nsite;isite1++){    
    for(isite2=1;isite2<=X->Def.Nsite;isite2++){
      is1_up=X->Def.Tpow[2*isite1-2];
      is1_down=X->Def.Tpow[2*isite1-1];
      is2_up=X->Def.Tpow[2*isite2-2];
      is2_down=X->Def.Tpow[2*isite2-1];


#pragma omp parallel for reduction(+:spn,spn_z) default(none) firstprivate(i_max, is1_up, is1_down, is2_up, is2_down, irght, ilft, ihfbit, isite1, isite2) private(ibit1_up, num1_up, ibit2_up, num2_up, ibit1_down, num1_down, ibit2_down, num2_down, tmp_spn_z, iexchg, off) shared(vec, list_1, list_2_1, list_2_2)
      for(j=1;j<=i_max;j++){                    

	ibit1_up= list_1[j]&is1_up;
	num1_up=ibit1_up/is1_up;            
	ibit2_up= list_1[j]&is2_up;
	num2_up=ibit2_up/is2_up;
            
	ibit1_down= list_1[j]&is1_down;
	num1_down=ibit1_down/is1_down;
	ibit2_down= list_1[j]&is2_down;
	num2_down=ibit2_down/is2_down;
            
	tmp_spn_z = (num1_up-num1_down)*(num2_up-num2_down);
	spn      += conj(vec[j])*vec[j]*tmp_spn_z/4.0;
	if(isite1==isite2){
	  spn+=conj(vec[j])*vec[j]*(num1_up+num1_down-2*num1_up*num1_down)/2.0;
	  spn_z    += conj(vec[j])*vec[j]*(num1_up-num1_down)/2.0;
	}else{
	  if(ibit1_up!=0 && ibit1_down==0 && ibit2_up==0 &&ibit2_down!=0 ){
	    iexchg= list_1[j]-(is1_up+is2_down);
	    iexchg+=(is2_up+is1_down);
	    GetOffComp(list_2_1, list_2_2, iexchg,  irght, ilft, ihfbit, &off);                    
	    spn+=conj(vec[j])*vec[off]/2.0;
	  }else if(ibit1_up==0 && ibit1_down!=0 && ibit2_up!=0 && ibit2_down==0){
	    iexchg= list_1[j]-(is1_down+is2_up);
	    iexchg+=(is2_down+is1_up);
	    GetOffComp(list_2_1, list_2_2, iexchg,  irght, ilft, ihfbit, &off);
	    spn+=conj(vec[j])*vec[off]/2.0;
	  }
	}
      }
    }
  }
  X->Phys.s2 = creal(spn);
  X->Phys.sz = creal(spn_z);
}


/** 
 * @brief function of calculating totalspin for Hubbard model at grand canonical ensemble
 * 
 * @param[in,out] X data list of calculation parameters
 * @param vec eigenvectors
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
void totalspin_HubbardGC(struct BindStruct *X,double complex *vec){ 
  long unsigned int j;
  long unsigned int isite1,isite2;
  long unsigned int is1_up,is2_up,is1_down,is2_down;
  long unsigned int iexchg, off;
  int num1_up,num2_up;
  int num1_down,num2_down; 
  long unsigned int ibit1_up,ibit2_up,ibit1_down,ibit2_down,list_1_j; 
  double complex spn_z,tmp_spn_z;
  double complex spn;
  long unsigned int i_max;
    
  i_max=X->Check.idim_max;

  spn   = 0.0;
  spn_z = 0.0;
  for(isite1=1;isite1<=X->Def.Nsite;isite1++){
    for(isite2=1;isite2<=X->Def.Nsite;isite2++){
      is1_up=X->Def.Tpow[2*isite1-2];
      is1_down=X->Def.Tpow[2*isite1-1];
      is2_up=X->Def.Tpow[2*isite2-2];
      is2_down=X->Def.Tpow[2*isite2-1];

#pragma omp parallel for reduction(+:spn,spn_z) default(none) firstprivate(i_max, is1_up, is1_down, is2_up, is2_down, isite1, isite2) private(list_1_j, ibit1_up, num1_up, ibit2_up, num2_up, ibit1_down, num1_down, ibit2_down, num2_down, tmp_spn_z, iexchg, off) shared(vec)
      for(j=1;j<=i_max;j++){                    
        list_1_j    = j-1;
	ibit1_up= list_1_j&is1_up;
	num1_up=ibit1_up/is1_up;            
	ibit2_up= list_1_j&is2_up;
	num2_up=ibit2_up/is2_up;
            
	ibit1_down= list_1_j&is1_down;
	num1_down=ibit1_down/is1_down;
	ibit2_down= list_1_j&is2_down;
	num2_down=ibit2_down/is2_down;
            
	tmp_spn_z = (num1_up-num1_down)*(num2_up-num2_down);
	spn+= conj(vec[j])*vec[j]*tmp_spn_z/4.0;
	if(isite1==isite2){
	  spn+=conj(vec[j])*vec[j]*(num1_up+num1_down-2*num1_up*num1_down)/2.0;
	  spn_z    += conj(vec[j])*vec[j]*(num1_up-num1_down)/2.0;
	}else{
	  if(ibit1_up!=0 && ibit1_down==0 && ibit2_up==0 &&ibit2_down!=0 ){
	    iexchg= list_1_j-(is1_up+is2_down);
	    iexchg+=(is2_up+is1_down);
            off    = iexchg+1;
	    spn+=conj(vec[j])*vec[off]/2.0;
	  }else if(ibit1_up==0 && ibit1_down!=0 && ibit2_up!=0 && ibit2_down==0){
	    iexchg= list_1_j-(is1_down+is2_up);
	    iexchg+=(is2_down+is1_up);
            off    = iexchg+1;
	    spn+=conj(vec[j])*vec[off]/2.0;
	  }
	}
      }
    }
  }
  X->Phys.s2 = creal(spn);
  X->Phys.sz = creal(spn_z);
}

/** 
 * @brief function of calculating totalspin for spin model
 * 
 * @param[in,out] X data list of calculation parameters
 * @param vec eigenvectors
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
void totalspin_Spin(struct BindStruct *X,double complex *vec){ 

  long unsigned int j;
  long unsigned int irght,ilft,ihfbit;
  long unsigned int isite1,isite2;
  long unsigned int is1_up,is2_up;
  long unsigned int iexchg, off, off_2;

  int num1_up,num2_up;
  int num1_down,num2_down; 
  int sigma_1, sigma_2;
  long unsigned int ibit1_up,ibit2_up,ibit_tmp,is_up; 
  double complex spn_z=0.0;
  double complex spn_z1, spn_z2;
  double complex spn=0.0;
  long unsigned int i_max;
    
  i_max=X->Check.idim_max;
  if(X->Def.iFlgGeneralSpin==FALSE){  
  GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, &irght, &ilft, &ihfbit);
  spn=0.0;
  spn_z=0.0;
  for(isite1=1;isite1<=X->Def.Nsite;isite1++){
    for(isite2=1;isite2<=X->Def.Nsite;isite2++){
      is1_up      = X->Def.Tpow[isite1-1];
      is2_up      = X->Def.Tpow[isite2-1];
      is_up       = is1_up+is2_up;
      
#pragma omp parallel for reduction(+: spn) default(none) firstprivate(i_max, is_up, is1_up, is2_up, irght, ilft, ihfbit, isite1, isite2) private(ibit1_up, num1_up, ibit2_up, num2_up, num1_down, num2_down, spn_z, iexchg, off, ibit_tmp) shared(list_1, list_2_1, list_2_2, vec)
      for(j=1;j<=i_max;j++){                    
	ibit1_up  = list_1[j]&is1_up;
	num1_up   = ibit1_up/is1_up;            
        num1_down = 1-num1_up;
	ibit2_up  = list_1[j]&is2_up;
	num2_up   = ibit2_up/is2_up;
        num2_down = 1-num2_up;
        
	spn_z  = (num1_up-num1_down)*(num2_up-num2_down);
	spn   += conj(vec[j])*vec[j]*spn_z/4.0;
            
	if(isite1==isite2){
	  spn      += conj(vec[j])*vec[j]/2.0;
	}else{
          ibit_tmp  = (num1_up) ^ (num2_up);
          if(ibit_tmp!=0){
            iexchg  = list_1[j] ^ (is_up);
            GetOffComp(list_2_1, list_2_2, iexchg, irght, ilft, ihfbit, &off);
	    spn    += conj(vec[j])*vec[off]/2.0;
	  }
	}
      }
    }
  }
  }
  else{
    double S1=0;
    double S2=0;
    for(isite1=1;isite1<=X->Def.Nsite;isite1++){
      for(isite2=1;isite2<=X->Def.Nsite;isite2++){
	S1=0.5*(X->Def.SiteToBit[isite1-1]-1);
	S2=0.5*(X->Def.SiteToBit[isite2-1]-1);
	if(isite1==isite2){
#pragma omp parallel for reduction(+: spn, spn_z) default(none) firstprivate(i_max, isite1, X, S1, spn_z1) shared(vec, list_1)	
	  for(j=1;j<=i_max;j++){
	    spn_z1  = 0.5*GetLocal2Sz(isite1, list_1[j], X->Def.SiteToBit, X->Def.Tpow);
	    spn    += conj(vec[j])*vec[j]*S1*(S1+1.0);
	    spn_z  += conj(vec[j])*vec[j]*spn_z1;
	  }
	}
	else{	  
#pragma omp parallel for reduction(+: spn) default(none) firstprivate(i_max, isite1, isite2, X, S1, S2) private(spn_z1, spn_z2, off, off_2, ibit_tmp, sigma_1, sigma_2) shared(vec, list_1)	
	  for(j=1;j<=i_max;j++){	  
	    spn_z1  = 0.5*GetLocal2Sz(isite1, list_1[j], X->Def.SiteToBit, X->Def.Tpow);
	    spn_z2  = 0.5*GetLocal2Sz(isite2, list_1[j], X->Def.SiteToBit, X->Def.Tpow);	  
	    spn    += conj(vec[j])*vec[j]*spn_z1*spn_z2;
	  
	    sigma_1=GetBitGeneral(isite1, list_1[j], X->Def.SiteToBit, X->Def.Tpow);
	    sigma_2=GetBitGeneral(isite2, list_1[j], X->Def.SiteToBit, X->Def.Tpow);
	  
	    ibit_tmp = GetOffCompGeneralSpin(list_1[j], isite2, sigma_2, sigma_2+1, &off, X->Def.SiteToBit, X->Def.Tpow);
	    if(ibit_tmp==TRUE){
	      ibit_tmp = GetOffCompGeneralSpin(off, isite1, sigma_1, sigma_1-1,&off_2, X->Def.SiteToBit, X->Def.Tpow);
	      if(ibit_tmp==TRUE) {
			  ConvertToList1GeneralSpin(off_2, X->Check.sdim, &off);
			  spn += conj(vec[j]) * vec[off] * sqrt(S2 * (S2 + 1) - spn_z2 * (spn_z2 + 1)) *
					 sqrt(S1 * (S1 + 1) - spn_z1 * (spn_z1 - 1)) / 2.0;
		  }
	    }
	  
	    ibit_tmp = GetOffCompGeneralSpin(list_1[j], isite2, sigma_2, sigma_2-1, &off, X->Def.SiteToBit, X->Def.Tpow);
	    if(ibit_tmp ==TRUE){
	      ibit_tmp = GetOffCompGeneralSpin(off, isite1, sigma_1, sigma_1+1, &off_2, X->Def.SiteToBit, X->Def.Tpow);
	      if(ibit_tmp==TRUE){
		ConvertToList1GeneralSpin(off_2, X->Check.sdim, &off);
		spn += conj(vec[j])*vec[off]*sqrt(S2*(S2+1) - spn_z2*(spn_z2-1.0))*sqrt(S1*(S1+1)- spn_z1*(spn_z1+1))/2.0;
	      }	  
	    }	  
	  }
	}
      }
    }
  }

  X->Phys.s2=creal(spn);
  X->Phys.sz=creal(spn_z);
}


/** 
 * @brief function of calculating totalspin for spin model at grand canonical ensemble
 * 
 * @param[in,out] X data list of calculation parameters
 * @param vec eigenvectors
 * @version 0.2
 * @details add function to treat a calculation of total spin for general spin
 *
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
void totalspin_SpinGC(struct BindStruct *X,double complex *vec){ 

  long unsigned int j;
  long unsigned int isite1,isite2;
  long unsigned int is1_up,is2_up;
  long unsigned int iexchg, off, off_2;
  int num1_up,num2_up;
  int num1_down,num2_down;
  int sigma_1, sigma_2;
  long unsigned int ibit1_up,ibit2_up,ibit_tmp,is_up; 
  double complex spn_z;
  double complex spn_z1, spn_z2;
  double complex spn;
  long unsigned int list_1_j;
  long unsigned int i_max;
    
  i_max=X->Check.idim_max;
  spn=0.0;
  spn_z=0.0;
  if(X->Def.iFlgGeneralSpin==FALSE){
    for(isite1=1;isite1<=X->Def.Nsite;isite1++){
      is1_up      = X->Def.Tpow[isite1-1];
#pragma omp parallel for reduction(+: spn_z) default(none) firstprivate(i_max, is1_up) private(list_1_j, ibit1_up, num1_up, num1_down) shared(vec)	
	for(j=1;j<=i_max;j++){
	   list_1_j=j-1;
	   ibit1_up  = list_1_j&is1_up;
	   num1_up   = ibit1_up/is1_up;            
	   num1_down = 1-num1_up;
	   spn_z  +=  conj(vec[j])*vec[j]*(num1_up-num1_down)/4.0;
	}      

	for(isite2=1;isite2<=X->Def.Nsite;isite2++){
	  is2_up      = X->Def.Tpow[isite2-1];
	  is_up       = is1_up+is2_up;
	
#pragma omp parallel for reduction(+: spn) default(none) firstprivate(i_max, is_up, is1_up, is2_up, isite1, isite2) private(list_1_j, ibit1_up, num1_up, ibit2_up, num2_up, num1_down, num2_down, spn_z2, iexchg, off, ibit_tmp) shared(vec)	
	for(j=1;j<=i_max;j++){
	  list_1_j=j-1;
	  ibit1_up  = list_1_j&is1_up;
	  num1_up   = ibit1_up/is1_up;            
	  num1_down = 1-num1_up;
	  ibit2_up  = list_1_j&is2_up;
	  num2_up   = ibit2_up/is2_up;
	  num2_down = 1-num2_up;
	  
	  spn_z2  = (num1_up-num1_down)*(num2_up-num2_down);
	  spn   += conj(vec[j])*vec[j]*spn_z2/4.0;
	  
	  if(isite1==isite2){
	    spn      += conj(vec[j])*vec[j]/2.0;
	  }else{	    	    
	    
	    ibit_tmp  = (num1_up) ^ (num2_up);
	    if(ibit_tmp!=0){
            iexchg  = list_1_j ^ (is_up);
            off    = iexchg+1;
	    spn    += conj(vec[j])*vec[off]/2.0;	    	    
	    }
	    
	  }
	}
      }
    }
  }
  else{
    double S1=0;
    double S2=0;
    spn_z=0;
    for(isite1=1;isite1<=X->Def.Nsite;isite1++){
      for(isite2=1;isite2<=X->Def.Nsite;isite2++){
	S1=0.5*(X->Def.SiteToBit[isite1-1]-1);
	S2=0.5*(X->Def.SiteToBit[isite2-1]-1);
	if(isite1==isite2){
#pragma omp parallel for reduction(+: spn, spn_z) default(none) firstprivate(i_max, isite1, X, S1) private(spn_z1) shared(vec)	
	  for(j=1;j<=i_max;j++){
	    spn_z1  = 0.5*GetLocal2Sz(isite1, j-1, X->Def.SiteToBit, X->Def.Tpow);
	    spn    += conj(vec[j])*vec[j]*S1*(S1+1.0);
	    spn_z += conj(vec[j])*vec[j]*spn_z1;
	  }
	}
	else{	  
#pragma omp parallel for reduction(+: spn) default(none) firstprivate(i_max, isite1, isite2, X, S1, S2) private(spn_z1, spn_z2, off, off_2, ibit_tmp, sigma_1, sigma_2) shared(vec)	
	  for(j=1;j<=i_max;j++){	  
	    spn_z1  = 0.5*GetLocal2Sz(isite1, j-1, X->Def.SiteToBit, X->Def.Tpow);
	    spn_z2  = 0.5*GetLocal2Sz(isite2, j-1, X->Def.SiteToBit, X->Def.Tpow);	  
	    spn    += conj(vec[j])*vec[j]*spn_z1*spn_z2;
	  
	    sigma_1=GetBitGeneral(isite1, j-1, X->Def.SiteToBit, X->Def.Tpow);
	    sigma_2=GetBitGeneral(isite2, j-1, X->Def.SiteToBit, X->Def.Tpow);
	  
	    ibit_tmp = GetOffCompGeneralSpin(j-1, isite2, sigma_2, sigma_2+1, &off, X->Def.SiteToBit, X->Def.Tpow);
	    if(ibit_tmp!=0){
	      ibit_tmp = GetOffCompGeneralSpin(off, isite1, sigma_1, sigma_1-1,&off_2, X->Def.SiteToBit, X->Def.Tpow);
	      if(ibit_tmp!=0){
		spn += conj(vec[j])*vec[off_2+1]*sqrt(S2*(S2+1) - spn_z2*(spn_z2+1))*sqrt(S1*(S1+1) - spn_z1*(spn_z1-1))/2.0;
	      }
	    }
	  
	    ibit_tmp = GetOffCompGeneralSpin(j-1, isite2, sigma_2, sigma_2-1, &off, X->Def.SiteToBit, X->Def.Tpow);
	    if(ibit_tmp !=0){
	      ibit_tmp = GetOffCompGeneralSpin(off, isite1, sigma_1, sigma_1+1, &off_2, X->Def.SiteToBit, X->Def.Tpow);
	      if(ibit_tmp!=0){
		spn += conj(vec[j])*vec[off_2+1]*sqrt(S2*(S2+1) - spn_z2*(spn_z2-1.0))*sqrt(S1*(S1+1)- spn_z1*(spn_z1+1))/2.0;
	      }
	  
	    }	  
	  }
	}
      }
    }
  }
  X->Phys.s2=creal(spn);
  X->Phys.sz=creal(spn_z);
}