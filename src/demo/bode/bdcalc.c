/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdcalc.c
 *
 */

/* Bode Plot calculation
 * Francisco Javier Gil Chica francisco.gil@ua.es *
*/

/*
la tabla contendra en la primera columna los logaritmos de
las frecuencias. En la segunda la propia frecuencia. En las
columnas sucesivas las sucesivas potencias

   G=P(s)/Q(s) * a(s)/b(s) * e(s)/f(s)
    ----------   ---------   ---------
     planta       retardo      PID

Formato de la tabla:
                                              delay orden 3 
                                              ---------------
   0    1   2    3    4    5    6    7    8    9    10    11    12    13    14       15
log(w), w, w^2, w^3, w^4, w^5, w^6, w^7, w^8, w^9, w^10, w^11, w^12, db(G), phn(G)   phd(G)
                                                               maxs  maxs+1  maxs+2  maxs+3
*/


#include "bdcalc.h"
#include "cassert.h"
#include "types.h"
#include "bmem.h"
#include "bmath.h"

#define ppd      100 /* puntos por década     */
#define decadas    4 /* decadas a representar */
#define maxs      12 /* maxima potencia de w  */
#define N        200 /* puntos para la grafica y(t) */

#define PI 3.14159265f

float _inferior=0.01f;  /* frecuencia inferior   */
float _tabla[decadas*ppd][maxs+4];

float _P[5];
float _Q[9];
float _K[3];

float _a[4];   /* para numerador delay */
float _b[4];   /* para denominador delay */

float _e[3];   /* numerador PID */
float _f[2];   /* denominador PID */

float _C[10];  /* delay*P*control  */
float _D[13];  /*  delay*Q*control  */
float _Z[13];  /* denominador bucle cerrado */

float _T;      /* periodo de simulacion */
float _R;      /* retardo */

float _evolucion[N][2];

/*---------------------------------------------------------------------------*/

static void precalcula(float inferior, float tabla[decadas*ppd][maxs+4])
{
    int indice=0,j,k;
    float h,g;

    for(j=0;j<decadas;++j){
        h=9*inferior/ppd;
        for(k=0;k<ppd;++k){
            g=inferior+k*h;
            tabla[indice][1]=g;
            tabla[indice][0]=bmath_log10f(g);
            ++indice;
        }
        inferior=inferior*10;
    }

    /* rellenar columnas */
    for(indice=0;indice<decadas*ppd;++indice){
        for(k=2;k<maxs+1;++k){
            tabla[indice][k]=tabla[indice][1]*tabla[indice][k-1];
        }
    }
    return;
}

/*---------------------------------------------------------------------------*/

static void retardo(const float r, float *a, float *b)
{
    a[0]=1.0f; a[1]=-r/2.0f; a[2]=r*r/10.0f; a[3]=r*r*r/120.0f;
    b[0]=1.0f; b[1]=-a[1];  b[2]=-a[2];    b[3]=-a[3];
    return;
}

/*---------------------------------------------------------------------------*/

static void FT(const float *P, const float *Q, 
               const float *a, const float *b, 
               float *e, float *f, 
               float *C, float *D, 
               float *Z,
               const float *K, float tabla[decadas*ppd][maxs+4])
{
    int indice, i, j, k;
    float ReC,ImC,ReD,ImD;
    float signo;
    
    for(i=0;i<10;++i)  C[i]=0.0f;
    for(i=0;i<13;++i)  D[i]=0.0f;

    if (K[0]!=0){
        e[0]=K[0];
        e[1]=K[1];
        e[2]=K[2];
        f[0]=0.0f;
        f[1]=1.0f;
    } else
    {
        e[0]=K[1];
        e[1]=K[2];
        e[2]=0.0f;
        f[0]=1.0f;
        f[1]=0.0f;
    }

    for (i=0;i<4;++i){
        for(j=0;j<5;++j){
            for(k=0;k<3;++k){
                C[i+j+k]+=a[i]*P[j]*e[k];
            }    
        }
    }
    for (i=0;i<4;++i){
        for(j=0;j<9;++j){
            for(k=0;k<2;++k){
                D[i+j+k]+=b[i]*Q[j]*f[k];
            }    
        }
    }

    /* denominador bucle cerrado */
    for(j=0;j<10;++j)
        Z[j]=C[j]+D[j];
    for(j=10;j<13;++j)
        Z[j]=0.0f;

    for(indice=0;indice<decadas*ppd;++indice){
        j=2; signo=-1.0f; ReC=C[0];
        while(j<10){
            ReC+=(signo*C[j]*tabla[indice][j]);
            j+=2;
            signo=-1.0f*signo;
        }
        j=1; signo=1.0f; ImC=0;
        while(j<10){
            ImC+=(signo*C[j]*tabla[indice][j]);
            j+=2;
            signo=-1.0f*signo;
        }

        j=2; signo=-1.0f; ReD=D[0];
        while(j<13){
            ReD+=(signo*D[j]*tabla[indice][j]);
            j+=2;
            signo=-1.0f*signo;
        }
        j=1; signo=1.0f; ImD=0;
        while(j<13){
            ImD+=(signo*D[j]*tabla[indice][j]);
            j+=2;
            signo=-1.0f*signo;
        }
        tabla[indice][maxs+1]=20*bmath_log10f(bmath_sqrtf(ReC*ReC+ImC*ImC))-20*bmath_log10f(bmath_sqrtf(ReD*ReD+ImD*ImD));
        tabla[indice][maxs+2]=bmath_atan2f(ImC,ReC)*180.0f/PI - 180.0f; /* paso a [0,-360] */
        tabla[indice][maxs+3]=bmath_atan2f(ImD,ReD)*180.0f/PI - 180.0f;
    }
    
    /* arreglar numerador */
    for(indice=1;indice<decadas*ppd;++indice){
        while (tabla[indice][maxs+2]-tabla[indice-1][maxs+2]>180.0f)
            tabla[indice][maxs+2]-=360;
        while (tabla[indice-1][maxs+2]-tabla[indice][maxs+2]>180.0f)
            tabla[indice][maxs+2]+=360;
    }
    /* arreglar denominador */
    for(indice=1;indice<decadas*ppd;++indice){
        while (tabla[indice][maxs+3]-tabla[indice-1][maxs+3]>180.0f)
            tabla[indice][maxs+3]-=360;
        while (tabla[indice-1][maxs+3]-tabla[indice][maxs+3]>180.0f)
            tabla[indice][maxs+3]+=360;
    }
    /* array final */
    for(indice=0;indice<decadas*ppd;++indice){
        /*printf("%6.3f %6.3f\n",tabla[indice][maxs+2],tabla[indice][maxs+3]);*/
        tabla[indice][maxs+2]-=tabla[indice][maxs+3];
    }
    
    return;
}

/*---------------------------------------------------------------------------*/

static int evo(const float *C, const float *Z, const float T, float evolucion[N][2])
{
    float x0[12];
    float x1[12];
    float kas[12][4];
    float h=T/(N-1);
    int i, j, maxj;
    float cu0,cut,cu;

    bmem_set_zero((byte_t*)x0, sizeof(x0));
    bmem_set_zero((byte_t*)x1, sizeof(x1));
    bmem_set_zero((byte_t*)kas, sizeof(kas));

    /* coeficientes segundo miembro */
    cu0=C[0]+C[1]/h;
    cut=C[0];

    /* encontrar máximo coeficiente de Z distinto de cero */
    maxj=12;
    while (Z[maxj]==0) --maxj;
    
    /* condiciones iniciales nulas */
    for(j=0;j<maxj;++j)
        x0[j]=0.0f;
    
    /* integracion mediante RK de cuarto orden */
    for(i=0;i<N;++i){
        if (i==0)
            cu=cu0;
        else
            cu=cut;

        /* primeros coeficientes */
        for(j=0;j<maxj-1;++j)
            kas[j][0]=h*x0[j+1];

        kas[maxj-1][0]=cu;
        for(j=0;j<maxj;++j)
            kas[maxj-1][0]=kas[maxj-1][0]-Z[j]*x0[j];
        kas[maxj-1][0]=(h/Z[maxj])*kas[maxj-1][0];    

        /* segundos coeficientes */
        for(j=0;j<maxj-1;++j)
            kas[j][1]=h*(x0[j+1]+0.5f*kas[j+1][0]);

        kas[maxj-1][1]=cu;
        for(j=0;j<maxj;++j)
            kas[maxj-1][1]=kas[maxj-1][1]-Z[j]*(x0[j]+0.5f*kas[j][0]);
        kas[maxj-1][1]=(h/Z[maxj])*kas[maxj-1][1];    

        /* terceros coeficientes */
        for(j=0;j<maxj-1;++j)
            kas[j][2]=h*(x0[j+1]+0.5f*kas[j+1][1]);

        kas[maxj-1][2]=cu;
        for(j=0;j<maxj;++j)
            kas[maxj-1][2]=kas[maxj-1][2]-Z[j]*(x0[j]+0.5f*kas[j][1]);
        kas[maxj-1][2]=(h/Z[maxj])*kas[maxj-1][2];    

        /* cuartos coeficientes */
        for(j=0;j<maxj-1;++j)
            kas[j][3]=h*(x0[j+1]+kas[j+1][2]);

        kas[maxj-1][3]=cu;
        for(j=0;j<maxj;++j)
            kas[maxj-1][3]=kas[maxj-1][3]-Z[j]*(x0[j]+kas[j][2]);
        kas[maxj-1][3]=(h/Z[maxj])*kas[maxj-1][3];    

        /* nuevos valores */
        for(j=0;j<maxj;++j){
            x1[j]=x0[j]+(1.0f/6.0f)*(kas[j][0]+2*kas[j][1]+2*kas[j][2]+kas[j][3]);
        }

        /* guardar tiempo y valor de y */
        evolucion[i][0]=i*h;
        evolucion[i][1]=x0[0];

        if ((evolucion[i][1]>100.0)||(evolucion[i][1]<-100.0))
            return(i);

        /* nuevas c.i. */
        for(j=0;j<maxj;++j){
            x0[j]=x1[j];
        }    
    }
    return(-1);
}

/*---------------------------------------------------------------------------*/

void bode_set_P_coeffs(const real32_t *p)
{
    bmem_copy_n(_P, p, 5, real32_t);
}

/*---------------------------------------------------------------------------*/

void bode_set_Q_coeffs(const real32_t *q)
{
    bmem_copy_n(_Q, q, 9, real32_t);
}

/*---------------------------------------------------------------------------*/

void bode_set_K(const real32_t *k)
{
    bmem_copy_n(_K, k, 3, real32_t);
}

/*---------------------------------------------------------------------------*/

void bode_set_T(const real32_t t)
{
    _T = t;
}

/*---------------------------------------------------------------------------*/

void bode_set_R(const real32_t r)
{
    _R = r;
}

/*---------------------------------------------------------------------------*/

void bode_update(uint32_t *last_sim_i)
{
    int i = -1;
    cassert_no_null(last_sim_i);
    *last_sim_i = UINT32_MAX;
    precalcula(_inferior, _tabla);
    retardo(_R, _a, _b);
    FT(_P, _Q, _a, _b, _e, _f, _C, _D, _Z, _K, _tabla);
    i=evo(_C, /*_D,*/ _Z, _T, _evolucion);
    
    if (i >= 0)
        *last_sim_i = (uint32_t)i;
}

/*---------------------------------------------------------------------------*/

uint32_t bode_npoints(void)
{
    return decadas*ppd;
}

/*---------------------------------------------------------------------------*/

void bode_db_graph(V2Df *v2d, const uint32_t n)
{
    register uint32_t i, total = min_u32(decadas * ppd, n);
    for (i = 0; i < total; ++i, v2d++)
    {
        v2d->x = _tabla[i][0];
        v2d->y = _tabla[i][maxs+1];
    }
}

/*---------------------------------------------------------------------------*/

void bode_phase_graph(V2Df *v2d, const uint32_t n)
{
    register uint32_t i, total = min_u32(decadas * ppd, n);
    for (i = 0; i < total; ++i, v2d++)
    {
        v2d->x = _tabla[i][0];
        v2d->y = _tabla[i][maxs+2];
    }
}

/*---------------------------------------------------------------------------*/

uint32_t bode_sim_npoints(void)
{
    return N;
}

/*---------------------------------------------------------------------------*/

void bode_sim_graph(V2Df *v2d, const uint32_t n)
{
    register uint32_t i, total = min_u32(N, n);
    for (i = 0; i < total; ++i, v2d++)
    {
        v2d->x = _evolucion[i][0];
        v2d->y = _evolucion[i][1];
    }
}

