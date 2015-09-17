/*	richfit.cc	*/
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
//#include <fstream>
#include <sstream>


#include "richfit.hh"
#include <math.h>
#include <algorithm>

namespace richanalysis {

void 
tensorIO::output_vector(gvec *v){
	std::cout <<"::INFO::VECTOR::";
	for(int i=0;i<v->size;i++)
		std::cout << " " << gsl_vector_get(v,i);
	std::cout << std::endl;
}

void 
tensorIO::output_matrix_label(gmat *M, gvec *v){
	std::cout <<"::INFO::MATRIX::"<< std::endl;
	for(int i=0;i<M->size2;i++){
		if(M->size2==v->size)
			std::cout << " " << gsl_vector_get(v,i);
		for(int j=0;j<M->size1;j++)
			std::cout << " " << gsl_matrix_get(M,j,i);
		std::cout << std::endl;	
	}
	std::cout << std::endl;
}

void 
tensorIO::output_matrix(gmat *M){
	std::cout <<"::INFO::MATRIX::"<< std::endl;
	for(int i=0;i<M->size2;i++){
		for(int j=0;j<M->size1;j++)
			std::cout << " " << gsl_matrix_get(M,j,i);
		std::cout << std::endl;	
	}
	std::cout << std::endl;
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

ftyp 
linalg::v_sum(gvec *v) {
	ftyp s=0.0;
	if(v->size>0)
		for(unsigned int i=0;i<v->size;i++)
			s+=gsl_vector_get(v,i);
	return s;
}

int 
linalg::calc_centroid( gmat *P, gvec *w, gvec *p0  ){
////	WEIGHTED CENTROID WHERE w IS NORMALIZED
	ftyp sum=0.0;
	for( unsigned int i=0 ; i<w->size ; i++)
		sum+=gsl_vector_get(w,i);
	if( p0->size == P->size1 && w->size == P->size2 ) {
		for( unsigned int i=0 ; i<p0->size ; i++ ){
			ftyp val=0.0;
			for( unsigned int j=0 ; j<w->size; j++ ){
				val+=gsl_matrix_get(P,i,j)*gsl_vector_get(w,j)/sum;
			}
			gsl_vector_set(p0,i,val);
		}
	}else{
		return 1;	
	}	

	return 0;
}

int 
linalg::calc_vmprod( gvec *w, gmat *P, gmat *wP){
	if( w->size == P->size2 && wP->size1 == P->size1 && wP->size2 == P->size2 ) {
		for( unsigned int i=0 ; i<P->size1; i++ ){
			for( unsigned int j=0 ; j<P->size2; j++ )
				gsl_matrix_set(wP, i, j, gsl_matrix_get(P,i,j)*gsl_vector_get(w,j));
		}
		return 0;
	}else{
		return 1;
	}
}

int 
linalg::center_matrix(gmat *P, gvec *p0){
////	CENTER COORDINATES P USING CENTROID p0
	if( P->size1 == p0->size && P->size2>0 ){
		for(unsigned int i=0;i<P->size1;i++)
			for(unsigned int j=0;j<P->size2;j++)
				gsl_matrix_set(P, i, j,
		 (gsl_matrix_get(P, i, j)-gsl_vector_get(p0, i)) );
		return 0;
	}else{
		return 1;
	}
}

int 
linalg::invert_matrix(gmat *A, gmat *invA) {
	int sign;
	gsl_permutation *p 	= gsl_permutation_alloc(A->size1);
	gsl_matrix *tmp_invA 	= gsl_matrix_alloc(A->size1, A->size2);
	gsl_matrix *tmpA 	= gsl_matrix_alloc(A->size1, A->size2);	
	gsl_matrix_memcpy(tmpA , A);

	gsl_linalg_LU_decomp (tmpA,p,&sign);
	gsl_linalg_LU_invert (tmpA,p,tmp_invA);
	gsl_matrix_memcpy(invA , tmp_invA);

	gsl_permutation_free(p);
	gsl_matrix_free(tmpA);
	gsl_matrix_free(tmp_invA);

	return sign;
}

ftyp 
linalg::get_det(gmat *A) {
	ftyp det=0.0;
	int signum;

	gsl_permutation *p = gsl_permutation_alloc(A->size1);

	gsl_matrix *tmpA = gsl_matrix_alloc(A->size1, A->size2);
	gsl_matrix_memcpy(tmpA , A);

	gsl_linalg_LU_decomp(tmpA , p , &signum);
	det = gsl_linalg_LU_det(tmpA , signum);
	gsl_permutation_free(p);
	gsl_matrix_free(tmpA);

	return det;
}
int	
quaternion::assign_quaterion( gvec *v, ftyp angle ){
// angle radians...
	if(is_complete() && v->size==3 ){
		ftyp norm,vx,vy,vz;
		gvec xo;
		vx=gsl_vector_get( v, XX );vy=gsl_vector_get( v, YY );vx=gsl_vector_get( v, ZZ );
  		norm   = 1.0/sqrt(vx*vx+vy*vy+vz*vz);

		gsl_vector_set ( q_ , XX ,  cos(angle*0.5) );
		gsl_vector_set ( q_ , YY , vx * norm * sin(angle*0.5) );
		gsl_vector_set ( q_ , ZZ , vy * norm * sin(angle*0.5) );
		gsl_vector_set ( q_ ,DIM , vz * norm * sin(angle*0.5) );

		return 0;
	}else {
		return 1;
	}
}

int
quaternion::rotate_coord( gvec *x ) 
// x the vector that is rotated
{
	ftyp xX,yY,zZ;

	if( is_complete() && x->size==DIM ){

		ftyp q[DIM+1],xo[DIM];
		q[XX] = gsl_vector_get(q_,XX); q[YY ] = gsl_vector_get(q_,YY );
		q[ZZ] = gsl_vector_get(q_,ZZ); q[DIM] = gsl_vector_get(q_,DIM);

		xo[XX] = gsl_vector_get(x, XX); 
		xo[YY] = gsl_vector_get(x, YY); 
		xo[ZZ] = gsl_vector_get(x, ZZ);
 
		xX = (q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3])*xo[XX] + (2*q[1]*q[2] - 2*q[0]*q[3])*xo[YY] + (2*q[1]*q[3] + 2*q[0]*q[2])*xo[ZZ];
		yY = (2*q[1]*q[2] + 2*q[0]*q[3])*xo[XX] + (q[0]*q[0]-q[1]*q[1] + q[2]*q[2]-q[3]*q[3])*xo[YY] + (2*q[2]*q[3]-2*q[0]*q[1])*xo[ZZ];
		zZ = (2*q[1]*q[3] - 2*q[0]*q[2])*xo[XX] + (2*q[2]*q[3] + 2*q[0]*q[1])*xo[YY] + (q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*xo[ZZ];

		gsl_vector_set(x,XX,xX);
		gsl_vector_set(x,YY,yY);
		gsl_vector_set(x,ZZ,zZ);

  		return 0;
	}else{
		return 1;
	}
}

ftyp 
fitting::shape_fit_tot(	gmat *P , gmat *Q ,	// IN
			gvec *w1, gvec *w2, 	// IN
			gmat *U , gvec *t ,	// OUT 
			int II ) {	// CASE VARIABLE (DETERMINANT)

	if( !(	   P->size1 == Q->size1 && P->size2 == w1->size
		&& U->size1 == U->size2 && U->size1 == P->size1 
		&& Q->size1 == U->size1 && Q->size2 == w2->size
		&& t->size  == P->size1 && P->size1 <= P->size2 ) )
		return(-1.0); 				// PROBLEMS WITH THE DIMENSIONS

	int 	L = ((int)P->size2),
		D = ((int)P->size1),
		LL= ((int)Q->size2);			// MATRIX DIMS

	if( !gsl_vector_ispos(w1) || !gsl_vector_ispos(w2) )
		return(-2.0); 				// PROBLEMS WITH VALUE

	ftyp wsum1 = v_sum(w1), wsum2 = v_sum(w2);
	if( wsum1 <= 0.0 || wsum2 <= 0.0 ) {
		return(-2.0); 				// PROBLEMS WITH VALUE
	} else if( !(wsum1 == 1.0 && wsum2 == 1.0) ) {	// THIS SHOULD BE DONE
		gsl_vector_scale(w1,1.0/wsum1);
		gsl_vector_scale(w2,1.0/wsum2);
	}
	gsl_vector *p0 = gsl_vector_alloc(D);
	gsl_vector *q0 = gsl_vector_alloc(D);

	calc_centroid(P,w1,p0); center_matrix(P,p0);
	calc_centroid(Q,w2,q0); center_matrix(Q,q0);

	gsl_matrix *w1P = gsl_matrix_alloc(P->size1,P->size2);
	calc_vmprod( w1, P, w1P );
	gsl_matrix *w2Q = gsl_matrix_alloc(Q->size1,Q->size2);
	calc_vmprod( w2, Q, w2Q );

	gsl_matrix *TMP = gsl_matrix_alloc( D, D );
	gsl_matrix *C   = gsl_matrix_alloc( D, D );
	gsl_matrix *V   = gsl_matrix_alloc( D, D );

	gsl_matrix *EYE = gsl_matrix_alloc( D, D );
	gsl_matrix_set_identity( EYE );
	ftyp det;

//	THE P AND Q INPUTS ARE STORED IN DxN MATRIX NOT NxD
	gsl_blas_dgemm(CblasNoTrans,CblasTrans, 1.0, w1P, w1P, 0.0, C);

////	THIS IS WHERE THE DECOMPOSITION OCCURS
	gsl_matrix *U1	= gsl_matrix_alloc( D, D );
	gsl_matrix *V1	= gsl_matrix_alloc( D, D );
	gsl_vector *S1	= gsl_vector_alloc( D );
	gsl_vector *wrk1= gsl_vector_alloc( D );

	gsl_linalg_SV_decomp ( C, V1, S1, wrk1 );
	gsl_matrix_memcpy( U1, C );	// HAVE ROTATION

	gsl_blas_dgemm(CblasNoTrans,CblasTrans, 1.0, w2Q, w2Q, 0.0, C);
//	AND THE SECOND SET
	gsl_matrix *U2	= gsl_matrix_alloc( D, D );
	gsl_matrix *V2	= gsl_matrix_alloc( D, D );
	gsl_vector *S2	= gsl_vector_alloc( D );
	gsl_vector *wrk2= gsl_vector_alloc( D );

	gsl_linalg_SV_decomp ( C, V2, S2, wrk2 );
	gsl_matrix_memcpy( U2, C );	// HAVE ROTATION

////	ROTATION	FOR THE MODEL (Q)
//	TAKE NOTE OF THE RESULTING SIGN ON U
	gsl_vector_memcpy(t, p0);
	switch(II){
		case  0: break;
		case  1: gsl_matrix_set  ( EYE,  0,  0, -1.0 ); gsl_matrix_set( EYE,  1,  1, -1.0 ); break;
		case  2: gsl_matrix_set  ( EYE,  0,  0, -1.0 ); gsl_matrix_set( EYE,  1,  1, -1.0 ); break;
		case  3: gsl_matrix_set  ( EYE,  2,  2, -1.0 ); gsl_matrix_set( EYE,  2,  2, -1.0 ); break;
		case  4: gsl_matrix_set  ( EYE,  0,  0, -1.0 ); break;
		case  5: gsl_matrix_set  ( EYE,  2,  2, -1.0 ); break;
		case  6: gsl_matrix_scale( EYE, -1.0); break;
		case  7: gsl_matrix_set  ( EYE,  1,  1, -1.0 ); break;
		default: break;
	}
	gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, EYE, V2, 0.0, TMP ); // CblasNoTrans, CblasTrans
	gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, V1, TMP, 0.0, C ); // CblasNoTrans, CblasNoTrans	
	gsl_matrix_memcpy( U , C );
	gsl_blas_dgemv( CblasNoTrans, -1.0, U, q0, 1.0, t );

	gsl_matrix_free(EYE);
	gsl_matrix_free(TMP);
	gsl_matrix_free( C );
	gsl_matrix_free( V );
	gsl_matrix_free(w1P);	gsl_matrix_free(w2Q);
	gsl_matrix_free(U1);	gsl_matrix_free(U2);
	gsl_matrix_free(V1);	gsl_matrix_free(V2);
	gsl_vector_free(S1);	gsl_vector_free(S2);
	gsl_vector_free(wrk1);	gsl_vector_free(wrk2);
	gsl_vector_free(p0);
	gsl_vector_free(q0);

	return (0.0);
}

void
fitting::invert_fit( gmat *U, gvec *t, gmat *invU, gvec *invt ) {
	if( invU->size1!=U->size1 || invU->size2!=U->size2 || t->size!=invt->size)
		std::cout << "ERROR IN DIMENSIONS"<< std::endl;
	invert_matrix(U,invU);
	gsl_blas_dgemv(CblasNoTrans, -1.0, invU, t, 0.0, invt);	
}


ftyp 
fitting::kabsch_fit(	gsl_matrix *P, gsl_matrix *Q, gsl_vector *w,	// IN
			gsl_matrix *U, gsl_vector *t ) {		// OUT
//// NOTE ON IO
//	IN 	P(D,M), Q(D,N), w(M)	
//	OUT	U(D,N), t(D), returns rmsd>0 or error<0
	if( !(	   P->size1 == Q->size1 && P->size2 == Q->size2
		&& U->size1 == U->size2 && U->size1 == P->size1 
		&& P->size2 == w->size  && t->size  == P->size1 ) )
		return(-1.0); 				// PROBLEMS WITH THE DIMENSIONS
	int L = ((int)P->size2), D = ((int)P->size1);	// STORED AS COLUMN MATRIX
	int LL= ((int)Q->size2);			// L CANNOT BE LARGER THAN LL
	if( !gsl_vector_ispos(w) )
		return(-2.0); 				// PROBLEMS WITH VALUE
	double wsum=v_sum(w);
	if( wsum <= 0.0 )
		return(-2.0); 				// PROBLEMS WITH VALUE
	gsl_vector_scale(w,1.0/wsum);
	gsl_vector *p0 = gsl_vector_alloc(D);
	gsl_vector *q0 = gsl_vector_alloc(D);

	calc_centroid(P,w,p0); center_matrix(P,p0);
	calc_centroid(Q,w,q0); center_matrix(Q,q0);

	gsl_matrix *C	= gsl_matrix_alloc(D,D);
	gsl_blas_dgemm(CblasNoTrans,CblasTrans, 1.0, P, Q, 0.0, C);

	gsl_matrix *W	= gsl_matrix_alloc( D, D );
	gsl_matrix *V	= gsl_matrix_alloc( D, D );
	gsl_vector *S	= gsl_vector_alloc( D );
	gsl_vector *work= gsl_vector_alloc( D );

	gsl_linalg_SV_decomp( C, W, S, work );
	gsl_matrix_memcpy( V, C );
	
	gsl_matrix *EYE = gsl_matrix_alloc( D, D );
	gsl_matrix *TMP = gsl_matrix_alloc( D, D );
	
	gsl_matrix_set_identity( EYE );
	
	gsl_blas_dgemm(CblasNoTrans,CblasTrans, 1.0, V, W, 0.0, C);
	double det = get_det(C);

	if (det < 0){	// FLIP IT!
		gsl_matrix_set(EYE,D-1,D-1,-1);
		gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, EYE, W, 0.0, TMP);
		gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, V, TMP, 0.0, C);
	}
	gsl_matrix_transpose_memcpy(U,C);

	gsl_blas_dgemv(CblasNoTrans, -1.0, U, p0, 1.0, q0);
	gsl_vector_memcpy(t,q0); 

	gsl_matrix *DIFF = gsl_matrix_alloc( D, L );

	double rmsd = 0.0;

	gsl_matrix_memcpy( DIFF, Q );
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans, 1.0, U, P, -1.0, DIFF);
	for( int i=0 ; i<L ; i++ ){
		rmsd += gsl_vector_get(w,i)*( 
				   square(gsl_matrix_get(DIFF,XX,i))
				 + square(gsl_matrix_get(DIFF,YY,i))
				 + square(gsl_matrix_get(DIFF,ZZ,i)) );
	}

	return sqrt(rmsd);
}

void 
fitting::apply_fit( gmat *M, gmat *U, gvec *t) {
	gsl_vector *pos = gsl_vector_alloc( 3 );
	gsl_vector *res = gsl_vector_alloc( 3 );
	int n = M->size1, m=M->size2;
	int L = n>m?n:m;

	for( int i=0 ; i<L ; i++ ){
		if(n>m){
			gsl_vector_set( pos, XX , gsl_matrix_get( M, i, XX ) );
			gsl_vector_set( pos, YY , gsl_matrix_get( M, i, YY ) );
			gsl_vector_set( pos, ZZ , gsl_matrix_get( M, i, ZZ ) );
		}else{
			gsl_vector_set( pos, XX , gsl_matrix_get( M, XX, i ) );
			gsl_vector_set( pos, YY , gsl_matrix_get( M, YY, i ) );
			gsl_vector_set( pos, ZZ , gsl_matrix_get( M, ZZ, i ) );
		}

		gsl_blas_dgemv( CblasNoTrans, 1.0, U, pos, 0.0, res );

		if(n>m){
			gsl_matrix_set(M,i,XX,gsl_vector_get(res,XX)+gsl_vector_get(t,XX));
			gsl_matrix_set(M,i,YY,gsl_vector_get(res,YY)+gsl_vector_get(t,YY));
			gsl_matrix_set(M,i,ZZ,gsl_vector_get(res,ZZ)+gsl_vector_get(t,ZZ));
		}else{
			gsl_matrix_set(M,XX,i,gsl_vector_get(res,XX)+gsl_vector_get(t,XX));
			gsl_matrix_set(M,YY,i,gsl_vector_get(res,YY)+gsl_vector_get(t,YY));
			gsl_matrix_set(M,ZZ,i,gsl_vector_get(res,ZZ)+gsl_vector_get(t,ZZ));
		}
	}
	gsl_vector_free(pos);
	gsl_vector_free(res);
}

void 
fitting::apply_fit( particles px, gmat *U, gvec *t) {
	gsl_vector *pos = gsl_vector_alloc( 3 );
	gsl_vector *res = gsl_vector_alloc( 3 );
	int L = ((int)px.size());

	for( int i=0 ; i<L ; i++ ){
		gsl_vector_set( pos, XX, gsl_vector_get( px[i].second,XX ) );
		gsl_vector_set( pos, YY, gsl_vector_get( px[i].second,YY ) );
		gsl_vector_set( pos, ZZ, gsl_vector_get( px[i].second,ZZ ) );

		gsl_blas_dgemv(CblasNoTrans, 1.0, U, pos, 0.0, res);

		gsl_vector_set( px[i].second , XX, gsl_vector_get(res,XX)+gsl_vector_get(t,XX) );
		gsl_vector_set( px[i].second , YY, gsl_vector_get(res,YY)+gsl_vector_get(t,YY) );
		gsl_vector_set( px[i].second , ZZ, gsl_vector_get(res,ZZ)+gsl_vector_get(t,ZZ) );
	}
	gsl_vector_free(pos);
	gsl_vector_free(res);
}

}
