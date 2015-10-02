/*	cluster.cc	*/
//C Copyright (C) 2015 Richard Tjörnhammar
//L
//L  This library is free software and is distributed under the terms
//L  and conditions of version 2.1 of the GNU Lesser General Public
//L  Licence (LGPL) with the following additional clause:
//L
//L     `You may also combine or link a "work that uses the Library" to
//L     produce a work containing portions of the Library, and distribute
//L     that work under terms of your choice, provided that you give
//L     prominent notice with each copy of the work that the specified
//L     version of the Library is used in it, and that you include or
//L     provide public access to the complete corresponding
//L     machine-readable source code for the Library including whatever
//L     changes were used in the work. (i.e. If you make changes to the
//L     Library you must distribute those, but you do not need to
//L     distribute source or object code to those portions of the work
//L     not covered by this licence.)'
//L
//L  Note that this clause grants an additional right and does not impose
//L  any additional restriction, and so does not affect compatibility
//L  with the GNU General Public Licence (GPL). If you wish to negotiate
//L  other terms, please contact the maintainer.
//L
//L  You can redistribute it and/or modify the library under the terms of
//L  the GNU Lesser General Public License as published by the Free Software
//L  Foundation; either version 2.1 of the License, or (at your option) any
//L  later version.
//L
//L  This library is distributed in the hope that it will be useful, but
//L  WITHOUT ANY WARRANTY; without even the implied warranty of
//L  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//L  Lesser General Public License for more details.
//L
//L  You should have received a copy of the CCP4 licence and/or GNU
//L  Lesser General Public License along with this library; if not, write
//L  to the CCP4 Secretary, Daresbury Laboratory, Warrington WA4 4AD, UK.
//L  The GNU Lesser General Public can also be obtained by writing to the
//L  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//L  MA 02111-1307 USA

#include <iostream>
#include "richtypes.h"
#include "cluster.hh"
#include <algorithm>

using namespace richanalysis;

void
cluster::alloc_space( int D1, int D2 ) {
//D1	COORDINATE	SPACE
//D2	CENTROID	SPACE
//	std::cout << "INFO::ALLOC::INFO "<< D1 << " " << D2 << " "<<DIM<<std::endl;
	if(D1>=D2){
		M_= gsl_matrix_calloc( DIM, D1 );
		bSet_[0] = 1;
		C_= gsl_matrix_calloc( DIM, D2 );
		bSet_[1] = 1;
		vc_	= gsl_vector_calloc( D1 );
		bSet_[2] = 1;
		wc_	= gsl_vector_calloc( D2 );
		bSet_[3] = 1;
	} else {
		std::cout << "ERROR IN ALLOCATION ROUTINE:: "<< D1 << ", " << D2 << std::endl;
	}
}

int
cluster::perform_clustering ( void ){
	if( bSet_[0] && bSet_[1] && bSet_[2] && bSet_[3] ) {
		gsl_kmeans(M_, vc_, C_, wc_);
		int N=wc_->size;
		int M=vc_->size;
		NperC_.clear();
		for(int i=0;i<N;i++){
			int numi=0;
			for( int j=0 ; j<vc_->size ; j++ ) {
				numi+=(gsl_vector_get(vc_,j)==i)?1:0;
			}
			NperC_.push_back(numi);
		}
	}
}

int
cluster::perform_clustering ( ftyp cutoff ){
	if( bSet_[0] && bSet_[1] && bSet_[2] && bSet_[3] ) {
		gsl_kmeans(M_, vc_, C_, wc_, cutoff);
		int N=wc_->size;
		int M=vc_->size;
		NperC_.clear();
		for(int i=0;i<N;i++){
			int numi=0;
			for( int j=0 ; j<vc_->size ; j++ ) {
				numi+=(gsl_vector_get(vc_,j)==i)?1:0;
			}
			NperC_.push_back(numi);
		}
	}
}

void
cluster::seM ( int i , int j , ftyp val){
	if(i<M_->size1&&j<M_->size2&&i>=0&&j>=0)
		gsl_matrix_set(M_,i,j,val);
}

void
cluster::seC ( int i , int j , ftyp val){
	if(i<C_->size1&&j<C_->size2&&i>=0&&j>=0)
		gsl_matrix_set(C_,i,j,val);
}

void
cluster::sew ( int i, ftyp val ) {
	if(i<wc_->size&&i>=0)
		gsl_vector_set(wc_,i,val);
}

void
cluster::sev ( int i, ftyp val ) {
	if(i<vc_->size&&i>=0)
		gsl_vector_set(vc_,i,val);
}

std::vector<int>	
cluster::get_labels( void ){
	std::vector<int> ndx;
	if( bSet_[0] && bSet_[1] && bSet_[2] && bSet_[3] )
		for(int i=0;i<vc_->size;i++)
			ndx.push_back(gsl_vector_get(vc_,i));
	return ndx;
}

int 
cluster::set_matrix( particles coord_space ) {
	int D = M_->size2;
	if( M_->size2 == coord_space.size() && M_->size1 == DIM ) {
		for(int i=0;i<D; i++){
			for(int j=XX;j<=ZZ;j++){
				seM( j, i, gsl_vector_get(coord_space[i].second,j) );
			}
		}
		gsl_vector_set_all ( vc_ , 1.0 ); // potentially set with label
		bSet_[0] = 1; bSet_[1] = 1;
		return  0;
	}else{
		std::cout << "ERROR:: BAD CONTAINER LENGTHS:: calc_distance_matrix" << std::endl;
		return -1;
	}
}

int 
clustering::connectivity(gsl_matrix *B, ftyp val) {
	int i,j,k,q,N,C=0,min;

	int nr_sq=B->size1;
	if(B->size1!=B->size2)
		return(-1);

	std::vector<int>	res;
	std::vector<int>	nvisi;

	std::vector<int>	s;
	std::vector<int>	NN;
	std::vector<int>	ndx;

	N=nr_sq;
	res.push_back(0);
	for(i=0; i<N; i++ ){
   		nvisi.push_back(i+1);
		res.push_back(0);res.push_back(0);
		ndx.push_back(i);
	}

	while(!ndx.empty()){
		i=ndx.back(); ndx.pop_back(); 
		NN.clear();

		if(nvisi[i]>0){
			C--;
			for(j=0;j<N;j++){
				if( gsl_matrix_get(B,j,i)<val==1 ) {
					NN.push_back(j);
				}
			}
			while(!NN.empty()){
				k=NN.back(); NN.pop_back();
				nvisi[k]=C;
				for(j=0;j<N;j++){
	   				if( gsl_matrix_get(B,j,k)<val==1 ){
	     					for(q=0;q<N;q++){
							if(nvisi[q]==j+1){
								NN.push_back(q);
							}
						}
					}
				}
			}
		}
	}
   
	std::cout <<"INFO " << C << "\ncluster data:\n";
	std::vector<int> Nc; // NUMBER OF POINTS IN EACH CLUSTER
	for(i=0;i<-1*C;i++)
		Nc.push_back(0);

	for(q=0;q<N;q++){
		res[q*2+1]=q;
		res[q*2]=nvisi[q]-C;
		Nc[res[q*2]]++;
		std::cout << " " << res[q*2] << " " << res[2*q+1] << std::endl ;
   	}
	// HOW MANY IN EACH?
	for(i=0;i<-1*C;i++)
		std::cout << "CLUSTER " << i << " HAS " << Nc[i] << " ELEMENTS" << std::endl ;

	return(0);   
}


int 
clustering::gsl_kmeans(gmat *dat, gsl_vector *w, gmat *cent, gsl_vector *nw ){
	int NN=dat->size2, MM=dat->size1, KK=cent->size2;

	gvec *labels = gsl_vector_alloc(NN);
	gvec *counts = gsl_vector_alloc(KK);
	gmat *tmp_ce = gsl_matrix_alloc(MM,KK);

	if( ((int)cent->size1)!=MM || !gsl_vector_isnonneg(w) || ((int)w->size)!=NN || ((int)nw->size)!=KK )	
		return -1;
	gsl_vector_set_zero(nw);

	int h, i, j;
	ftyp old_error, error = 1E30, TOL=1E-8; 

	std::vector<int> myvector;
	for (i=0; i<NN; ++i) myvector.push_back(i); 
	random_shuffle ( myvector.begin(), myvector.end() );

	i=0;
	for ( h=0 ; h<KK ; h++ ){
		for ( j=XX ; j<=ZZ ; j++ ){ 
			gsl_matrix_set( cent, j, h , gsl_matrix_get( dat, j, myvector[h] ) );
		} 
	}
	do {
		old_error = error, error = 0; 
		for (i = 0; i < KK; i++ ) {
			gsl_vector_set(counts,i,0);
			for (j = XX; j <= ZZ; j++){
				gsl_matrix_set(tmp_ce,j,i,0.0);
			}
 		}
		for (h = 0; h < NN; h++) {
			ftyp min_distance = 1E30;
			for (i = 0; i < KK; i++) {
				ftyp distance = 0;
				for ( j = XX; j<=ZZ ; j++ ) {
					distance += square( gsl_matrix_get( dat,j,h ) - gsl_matrix_get( cent,j,i ) );
				}
				if (distance < min_distance) {
	 				gsl_vector_set(labels,h,i); min_distance = distance; 
				} 
			} 
			for ( j=XX ; j<=ZZ ; j++ ){
 				gsl_matrix_set( tmp_ce, j, gsl_vector_get(labels,h),
				 gsl_matrix_get( dat, j, h ) + gsl_matrix_get(tmp_ce, j, gsl_vector_get(labels,h)) );
			}
			gsl_vector_set(counts,gsl_vector_get(labels,h),1.0+gsl_vector_get(counts,gsl_vector_get(labels,h)));
	 		error += min_distance; 
		}
	 	for (i = 0; i < KK; i++) {
	 		for ( j=XX ; j<=ZZ ; j++ ) {
				gsl_matrix_set(cent,j,i,
				gsl_vector_get(counts,i)?(gsl_matrix_get(tmp_ce,j,i)/gsl_vector_get(counts,i)):(gsl_matrix_get(tmp_ce,j,i)));
	 		}
	 	}
	} while ( fabs(error - old_error) > TOL );	// WHILE THEY ARE MOVING

	ftyp wi=0.0, nwh=0.0;
	for( i=0 ; i<NN ; i++) {
		h 	= gsl_vector_get(labels,i);
		wi	= gsl_vector_get(w,i);
		nwh	= gsl_vector_get(nw,h);
		gsl_vector_set(w,i,h);			// MIGHT NOT WANT TO OVERWRITE THIS
		gsl_vector_set(nw,h,nwh+wi); 		// NOT NORMALIZED HERE
	}

	gsl_vector_free(labels); 
	gsl_vector_free(counts);
	gsl_matrix_free(tmp_ce);

	return 0;
}


int 
clustering::gsl_kmeans(gmat *dat, gsl_vector *w, gmat *cent, gsl_vector *nw, ftyp cutoff){
	int NN=dat->size2, MM=dat->size1, KK=cent->size2;

	gvec *labels = gsl_vector_alloc(NN);
	gvec *counts = gsl_vector_alloc(KK);
	gmat *tmp_ce = gsl_matrix_alloc(MM,KK);

	if( ((int)cent->size1)!=MM || !gsl_vector_isnonneg(w) || ((int)w->size)!=NN || ((int)nw->size)!=KK )	
		return -1;
	gsl_vector_set_zero(nw);

	int h, i, j;
	ftyp old_error, error = 1E30, TOL=1E-4; 

	std::vector<int> myvector;
	for (i=0; i<NN; ++i) myvector.push_back(i); 
	random_shuffle ( myvector.begin(), myvector.end() );

	i=0;
	for ( h=0 ; h<KK ; h++ ){
		for ( j=XX ; j<=ZZ ; j++ ){ 
			gsl_matrix_set( cent, j, h , gsl_matrix_get( dat, j, myvector[h] ) );
		} 
	}
	do {
		old_error = error, error = 0; 
		for (i = 0; i < KK; i++ ) {
			gsl_vector_set(counts,i,0);
			for (j = XX; j <= ZZ; j++){
				gsl_matrix_set(tmp_ce,j,i,0.0);
			}
 		}
		for (h = 0; h < NN; h++) {
			ftyp min_distance = 1E30;
			for (i = 0; i < KK; i++) {
				ftyp distance = 0;
				for ( j = XX; j<=ZZ ; j++ ) {
					distance += square( gsl_matrix_get( dat,j,h ) - gsl_matrix_get( cent,j,i ) );
				}
				if ( distance < min_distance   ) { // && distance > square(cutoff)
	 				gsl_vector_set(labels,h,i); min_distance = distance; 
				} 
			} 
			for ( j=XX ; j<=ZZ ; j++ ){
 				gsl_matrix_set( tmp_ce, j, gsl_vector_get(labels,h),
				 gsl_matrix_get( dat, j, h ) + gsl_matrix_get(tmp_ce, j, gsl_vector_get(labels,h)) );
			}
			gsl_vector_set(counts,gsl_vector_get(labels,h),1.0+gsl_vector_get(counts,gsl_vector_get(labels,h)));
	 		error += min_distance; 
		}
	 	for (i = 0; i < KK; i++) {
	 		for ( j=XX ; j<=ZZ ; j++ ) {
				gsl_matrix_set(cent,j,i,
				gsl_vector_get(counts,i)?(gsl_matrix_get(tmp_ce,j,i)/gsl_vector_get(counts,i)):(gsl_matrix_get(tmp_ce,j,i)));
	 		}
	 	}
	} while ( fabs(error - old_error) > TOL );	// WHILE THEY ARE MOVING

	ftyp wi=0.0, nwh=0.0;
	for( i=0 ; i<NN ; i++) {
		h 	= gsl_vector_get(labels,i);
		wi	= gsl_vector_get(w,i);
		nwh	= gsl_vector_get(nw,h);
		gsl_vector_set(w,i,h);			// MIGHT NOT WANT TO OVERWRITE THIS
		gsl_vector_set(nw,h,nwh+wi); 		// NOT NORMALIZED HERE
	}

	gsl_vector_free(labels); 
	gsl_vector_free(counts);
	gsl_matrix_free(tmp_ce);

	return 0;
}

std::vector<int>			
cluster_node::find_simple_relation( void ){
	std::vector<int> residx;
	if(!subSet()){
		std::cout << "INFO::SUBNOTSET::" << std::endl;
		return residx;
	}
	int N = vc1_.size();
	int M = vc2_.size();
	std::cout << "INFO:: HAVE SUBCLUSTERS OF SIZE::" << N << " AND " << M << std::endl;
	int Lt[2]={0,0};
	std::vector<int> unassigned;
	for(int i=0;i<N;i++)
	{
		Lt[0]+=vc1_[i].length_M();
	}

	for(int j=0;j<M;j++)
	{
		Lt[1]+=vc2_[j].length_M();
		unassigned.push_back(j);
	}
	for(int i=0;i<N;i++) {
		ftyp rmsd=1E10;
		ftyp frac0=(float)vc1_[i].length_M()/(float)Lt[0];
		int J=-1,JJ=-1;
		for(int j=0;j<unassigned.size();j++) {
			J=unassigned[j];
			ftyp val=square(frac0-(float)vc2_[J].length_M()/(float)Lt[1]);
			if(rmsd>val)
			{
				JJ=J;
				rmsd=val;
			}
		}
		residx.push_back(JJ);
		if(residx.size()<N)
			unassigned.erase(unassigned.begin()+JJ);
	}
/*
			gmat *P = gsl_matrix_calloc(DIM,vc1_[i].length_M());
			vc1_[i].copyM(P);
			gmat *Q = gsl_matrix_calloc(DIM,vc2_[j].length_M());
			vc2_[j].copyM(Q);
			gmat *U = gsl_matrix_calloc(DIM,DIM);
			gvec *t = gsl_vector_calloc(DIM);

			double rmsd=shape_fit( P, Q, U, t );
			std::cout << "INFO::HAVE::RMSD::" << rmsd << " i " << i << " j " << j 
				<< " L1 " << vc1_[i].length_M() << " L2 " << vc2_[j].length_M() << std::endl;
			if( rmsd<rmsd0 ) {
				rmsd0=rmsd;
				if(residx.size()==i+1){
					residx[i]=j;
				}else{
					residx.push_back(j);
				}
			}

		}
	}
*/	std::cout << "INFO::HAVE::N_IDX::" << residx.size() << std::endl;
	for(int i=0;i<residx.size();i++)
		std::cout << "INFO:: " << residx[i] << std::endl;
	
}

void
cluster_node::assign_sub( cluster c1, cluster c2 ) {

	int N,M;
	if(c1.isSet()&&c2.isSet()){

		N=c1.length_M();
		M=c1.length_C();
		gmat *M1 = gsl_matrix_calloc(DIM,N);
		gvec *v1 = gsl_vector_calloc(N);
		c1.copyM(M1); c1.copyv(v1);
		for( int ipart=0; ipart<M; ipart++ ){
			richanalysis::coord_format cf;
			particles px;
			px	= cf.truncmat(M1,v1,ipart);
			int D	= px.size();
			richanalysis::cluster clpx;
			clpx.alloc_space( D, 3 );	// THESE CENTOIDS SHOULD NOT BE USED
			clpx.set_matrix( px );
			clpx.perform_clustering();
			vc1_.push_back(clpx);
		}

		N=c2.length_M();
		M=c1.length_C();
		gmat *M2 = gsl_matrix_calloc(DIM,N);
		gvec *v2 = gsl_vector_calloc(N);
		c2.copyM(M2); c2.copyv(v2);
		for( int ipart=0; ipart<M; ipart++ ){
			richanalysis::coord_format cf;
			particles px;
			px	= cf.truncmat(M2,v2,ipart);
			int D	= px.size();
			richanalysis::cluster clpx;
			clpx.alloc_space( D, 3 );	// THESE CENTOIDS SHOULD NOT BE USED
			clpx.set_matrix( px );
			clpx.perform_clustering();
			vc2_.push_back(clpx);
		}
		subSet_=1;

	}
}

particles 
cluster_node::apply_rot_trans( particles px , int I){
	if(bUtSet_){
		if(I>=0)
			apply_fit( px, U_, t_	);
		if(I<0)
			apply_fit( px, iU_, it_	);
	}
	return px;
}

particles 
cluster_node::apply_rot_trans( particles px ){
	if(bUtSet_){
		if(sgn_>=0)
			apply_fit( px, U_, t_	);
		if(sgn_<0)
			apply_fit( px, iU_, it_	);
	}
	return px;
}

void
cluster_node::invert_transform( void ){
	if( iU_->size1==U_->size1 && iU_->size2==U_->size2 && t_->size==it_->size){
		invert_matrix(U_,iU_);
		gsl_blas_dgemv(CblasNoTrans, -1.0, iU_, t_, 0.0, it_);
	}
}

ftyp
cluster_node::find_centroid_relation(cluster c1, cluster c2){
	ftyp min_rmsd	= 1.0E10;

	int N = c1.length_C();

	if(N != c2.length_C())
		std::cout <<"ERROR IN IDX REL"<<std::endl;

	gmat *U		= gsl_matrix_calloc( DIM, DIM );
	gvec *t		= gsl_vector_calloc( DIM );

	int J		= 0;

	gmat *C0T	= gsl_matrix_calloc( DIM, N );
	gmat *C0N	= gsl_matrix_calloc( DIM, N );
	gmat *CNT	= gsl_matrix_calloc( DIM, N );
	gmat *CEN	= gsl_matrix_calloc( DIM, N );
	gvec *gv 	= gsl_vector_calloc( DIM );

	c1.copyC(C0N);
	c2.copyC(C0T);

	gmat *Ut = gsl_matrix_calloc( DIM, DIM );
	gvec *tt = gsl_vector_calloc( DIM );

	std::vector<int> iv;
	for(int i=0;i<N;i++)
		iv.push_back(i);
	std::vector<std::vector<int> > imv = all_permutations(iv);

	for(int j=0;j<imv.size();j++){
		gsl_matrix_memcpy (CEN, C0N);
		for(int i=0;i<iv.size();i++) {
			gsl_matrix_get_col (gv, C0T, i);
			gsl_matrix_set_col (CNT, imv[j][i], gv );
		}
		ftyp rmsd = kabsch_fit(CNT,CEN,U,t);
		if(rmsd<min_rmsd){
			J=j; 
			min_rmsd=rmsd;	
			gsl_matrix_memcpy (Ut, U);
			gsl_vector_memcpy (tt, t);
		}
	}

	if(idx_.size()==0) { 
		for(int i=0;i<iv.size();i++) // THIS IS WHAT WE WANTED
			idx_.push_back(imv[J][i]);
	} else {
		std::cout << "INFO::ERROR WITH IDX_" << std::endl;
		idx_.clear();
		for(int i=0;i<iv.size();i++)
			idx_.push_back(imv[J][i]);
	}

	bDirRel_=2*(c1.length_M()>c2.length_M())-1;

//	if(!have_transform())
	{
		gsl_matrix_memcpy (U_, Ut);
		gsl_vector_memcpy (t_, tt);
		invert_transform();
		bUtSet_=1;
		sgn_=1;
	}

	gsl_matrix_free(CNT);
	gsl_matrix_free(C0N);
	gsl_matrix_free(C0T);
	gsl_matrix_free(U);
	gsl_vector_free(t);
	gsl_vector_free(gv);

	return min_rmsd;
}

ftyp
cluster::find_shape(){
//	Do a single shape fit

	if( isSet() ) { 
		int N 		= length_M();
		gmat *MEN	= gsl_matrix_calloc( DIM, N );
		copyM(MEN);
	
		gmat *Utmp	= gsl_matrix_calloc( DIM, DIM );
		gvec *ttmp	= gsl_vector_calloc( DIM );
		
		ftyp min_rmsd 	= shape(MEN,Utmp,ttmp);

		gsl_matrix_memcpy (Uc_, Utmp);
		gsl_vector_memcpy (tc_, ttmp);
		bPCset_		=	1;

		gsl_matrix_free(MEN);
		gsl_matrix_free(Utmp); 
		gsl_vector_free(ttmp);
	}

	return 0.0;
}


ftyp 
cluster::shape(		gmat *P, gmat *U , gvec *t 	) 
{	
	if( !(	   P->size1 
		&& U->size1 == U->size2 && U->size1 == P->size1 
		&& t->size  == P->size1 && P->size1 <= P->size2 ) )
		return(-1.0); 		// PROBLEMS WITH THE DIMENSIONS

	int 	L = ((int)P->size2),
		D = ((int)P->size1);	// MATRIX DIMS

	ftyp wsum = P->size2;
	if( wsum <= 0.0 ) {
		return(-2.0); 		// PROBLEMS WITH VALUE
	}

	gsl_vector *w = gsl_vector_alloc((int)P->size2);
	gsl_vector_set_all( w , 1.0/wsum );
	gsl_vector *p0 = gsl_vector_alloc(D);

	calc_centroid(P,w,p0); center_matrix(P,p0);
	gsl_matrix *wP = gsl_matrix_alloc(P->size1,P->size2);
	calc_vmprod( w, P, wP );

	gsl_matrix *W	= gsl_matrix_alloc( L, D );
	gsl_matrix *V   = gsl_matrix_alloc( D, D );
	gsl_matrix *C   = gsl_matrix_alloc( L, D );
	gsl_matrix_transpose_memcpy (C, wP);
	gsl_vector *S	= gsl_vector_alloc( D );
	gsl_vector *wrk = gsl_vector_alloc( D );

	gsl_matrix *TMP = gsl_matrix_alloc( D, D );
	gsl_matrix *EYE = gsl_matrix_alloc( D, D );

	gsl_linalg_SV_decomp ( C, V, S, wrk );
	gsl_matrix_memcpy( W, C );

	output_vector(S);

	gsl_matrix_transpose_memcpy(U,V);
	gsl_vector_memcpy(t, p0);

	gsl_vector_free(w);
	gsl_vector_free(p0);	
	gsl_matrix_free( wP);	
	gsl_matrix_free( W );	
	gsl_matrix_free( V );
	gsl_matrix_free( C );
	gsl_vector_free( S );	
	gsl_vector_free(wrk);	
	gsl_matrix_free(TMP);		
	gsl_matrix_free(EYE);	

	return 0.0;
}

particle
cluster::normal( void ){
	particle par;
	if( has_shape() ){
		gmat *Uc1	= gsl_matrix_calloc(DIM,DIM);
		gvec *tc	= gsl_vector_calloc(DIM);
		copyUc(Uc1);
		copytc(tc);
		gvec *a = gsl_vector_calloc(DIM);
 		gsl_matrix_get_row (a, Uc1, ZZ);
		par.first ="Ag";
		gsl_vector_add(a,tc);
		par.second=a;
		return par;
	}else{
		return par;
	}
}
particle
cluster::center( void ){
	if( has_shape() ){
		gvec *tc	= gsl_vector_calloc(DIM);
		copytc(tc);
		particle par;
		par.first ="Ag";
		par.second=tc;
		return par;
	}
}

ftyp			
cluster_node::angle_between(cluster c1, cluster c2){

	if( c1.has_shape() && c2.has_shape() ){
		ftyp a,b,c,x,y,z;
		gmat *Uc1 = gsl_matrix_calloc(DIM,DIM);
		c1.copyUc(Uc1);
		a = gsl_matrix_get(Uc1,XX,ZZ); 
		b = gsl_matrix_get(Uc1,YY,ZZ); 
		c = gsl_matrix_get(Uc1,ZZ,ZZ);
		gmat *Uc2 = gsl_matrix_calloc(DIM,DIM);
		c2.copyUc(Uc2);
		x = gsl_matrix_get(Uc2,XX,ZZ); 
		y = gsl_matrix_get(Uc2,YY,ZZ); 
		z = gsl_matrix_get(Uc2,ZZ,ZZ);

		gvec *tc1	= gsl_vector_calloc(DIM);
		c1.copytc(tc1);
		gvec *tc2	= gsl_vector_calloc(DIM);
		c2.copytc(tc2);
		gsl_vector_sub(tc2,tc1);

		ftyp dx = gsl_vector_get(tc2,XX);
		ftyp dy = gsl_vector_get(tc2,YY);
		ftyp dz = gsl_vector_get(tc2,ZZ);

		ftyp dlen0=1.0/sqrt(dx*dx+dy*dy+dz*dz);
		ftyp ilen1=1.0/sqrt(a*a+b*b+c*c);
		ftyp ilen2=1.0/sqrt(x*x+y*y+z*z);

		ftyp angle=atan2( (dx*(b*z-c*y)+dy*(c*x-a*z)+dz*(a*y-b*x))*dlen0*ilen1*ilen2 , (a*x+b*y+c*z)*ilen1*ilen2 );

		gsl_matrix_free(Uc1); gsl_matrix_free(Uc2);
		std::cout << "INFO::TORSION  " << angle*180/M_PI << std::endl;
		return angle;
	}else{
		return -1000.0;
	}
}


std::pair<ftyp,ftyp >			
cluster_node::angle_between(cluster c1, cluster c2, int I, int J){

	std::pair<ftyp,ftyp > ang_dist;

	if( c1.has_shape() && c2.has_shape() ){
		ftyp a,b,c,x,y,z;
		gmat *Uc1 = gsl_matrix_calloc(DIM,DIM);
		gmat *Uc2 = gsl_matrix_calloc(DIM,DIM);
		gvec *tc1 = gsl_vector_calloc(DIM);
		gvec *tc2 = gsl_vector_calloc(DIM);
		c1.copyUc(Uc1);
		c2.copyUc(Uc2);
		c1.copytc(tc1);
		c2.copytc(tc2);
		gsl_vector_sub(tc1,tc2);
		{ 
			a = gsl_matrix_get(Uc1,XX,ZZ); 
			b = gsl_matrix_get(Uc1,YY,ZZ); 
			c = gsl_matrix_get(Uc1,ZZ,ZZ);

			x = gsl_matrix_get(Uc2,XX,ZZ); 
			y = gsl_matrix_get(Uc2,YY,ZZ); 
			z = gsl_matrix_get(Uc2,ZZ,ZZ);

			ftyp dx = gsl_vector_get(tc1,XX);
			ftyp dy = gsl_vector_get(tc1,YY);
			ftyp dz = gsl_vector_get(tc1,ZZ);

			ftyp dlen0=1.0/sqrt(dx*dx+dy*dy+dz*dz);
			ftyp ilen1=1.0/sqrt(a*a+b*b+c*c);
			ftyp ilen2=1.0/sqrt(x*x+y*y+z*z);

			ftyp angle	= atan2( (dx*(b*z-c*y)+dy*(c*x-a*z)+dz*(a*y-b*x))*dlen0*ilen1*ilen2 , (a*x+b*y+c*z)*ilen1*ilen2 );
			ang_dist.first	= angle*180.0/M_PI;
			ang_dist.second	= 1.0/dlen0;
		}
		gsl_vector_free(tc1); gsl_vector_free(tc2);
		gsl_matrix_free(Uc1); gsl_matrix_free(Uc2);
	}else{
		ang_dist.first=0.0; ang_dist.second=-1.0;
	}
	return ang_dist;
}


ftyp
cluster_node::find_shape_trans( cluster c1, cluster c2 ){
//	Do a single shape fit

	int N = c1.length_M();
	gmat *MEN	= gsl_matrix_calloc( DIM, N );
	c1.copyM(MEN);

	int M = c2.length_M();
	gmat *MNT	= gsl_matrix_calloc( DIM, M );
	c2.copyM(MNT);

	gmat *U		= gsl_matrix_calloc( DIM, DIM );
	gvec *t		= gsl_vector_calloc( DIM );

	ftyp min_rmsd = shape_fit(MNT,MEN,U,t);

	gsl_matrix_memcpy (U_, U);
	gsl_vector_memcpy (t_, t);
	invert_transform();
	bUtSet_=1;
	sgn_=1;

	gsl_matrix_free(MNT);
	gsl_matrix_free(MEN);

	gsl_matrix_free(U); 
	gsl_vector_free(t);

	return min_rmsd;
}
