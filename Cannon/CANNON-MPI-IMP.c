/*********************************************************************************************
 * Archivo: CANNON-MPI.c
 *
 * Implementacion del algoritmo de Cannon para la multiplicacion de matrices
 * densas utilizando la Interfaz de Paso de Mensajes MPI.
 *
 * Para compilar:
 * gcc CANNON-MPI-IMP.c -o CANNON-MPI -I c:\ap -L c:\ap -lmpich
 *
 * Autores:
 * - Eduardo Rivas. erivas17@gmail.com
 * - Hugo Meyer. meyer.hugo@gmail.com
 *
 * Alumnos de la Universidad Nacional de Asuncion - Facultad Politecnica.
 * Carrera: Ingenieria Informatica.
 * Materia: Electiva V - Algoritmos Paralelos.
 * Profesor: Cristian Von Lucken.
 * Anho: 2008.
 **********************************************************************************************/
#include <stdio.h>
#include "mpi.h"
#define n 4
#include <math.h>

int mi_id, rank_envio;
float A[n][n], B[n][n], C[n][n];
int coords[2], dims[2], periodos[2], coords_envio[2];

void llenarMatriz(float m[n][n])
{
  static float k=0;
  int i, j;
  for (i=0; i<n; i++)
    for (j=0; j<n; j++)
      m[i][j] = k++;
}

void imprimirMatriz(float m[n][n])
{
  int i, j = 0;
  for (i=0; i<n; i++) {
    printf("\n\t| ");
    for (j=0; j<n; j++)
      printf("%2f ", m[i][j]);
    printf("|");
  }
}
void imprimirSubMatriz(float m[3][3])
{
  int i, j = 0;
  for (i=0; i<3; i++) {
    printf("\n\t| ");
    for (j=0; j<3; j++)
      printf("%2f ", m[i][j]);
    printf("|");
  }
}
int main(int argc, char** argv) {
    /*DEFINICIONES DE DATOS E INICIALIZACIONES*/
    int mi_fila, mi_columna, fila_recepcion, col_recepcion;
    double timeIni, timeFin;
    int rank_envio,size,destino,fuente;
    int i,j,k,l, cont_fila, cont_columna, ciclos;
    FILE *fp;
    MPI_Status statusA;
    MPI_Status statusB;
    MPI_Status statusC;

	MPI_Comm comm2d;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&mi_id);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

	double m = pow((double)size,(double) 1/2);
	if (n % (int)m !=0 )
    {
        printf("Por favor corra con una cantidad de procesos multiplo de %d.\n", n);fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

	int tam_subM = n/m;
    dims[0]=dims[1]=(int) m;
    periodos[0]=periodos[1]=1;

    float subm_A[tam_subM][tam_subM];
    float subm_B[tam_subM][tam_subM];
    float subm_C[tam_subM][tam_subM];
    float subm_C_aux[tam_subM][tam_subM];
    /*EN ESTE CASO SOLO ES DE DOS DIMENSIONES*/
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodos, 0, &comm2d);
    /* Obtiene mi nuevo id en 2D */
    MPI_Comm_rank(comm2d, &mi_id);
    /* Obtiene mis coordenadas */
    MPI_Cart_coords(comm2d, mi_id, 2, coords);

    mi_fila = coords[0];
    mi_columna= coords[1];

    /*inicializamos submatriz C*/
    for (i=0; i<tam_subM; i++){
        for (j=0; j<tam_subM; j++){
            subm_C[i][j] =0;
        }
    }

    if(mi_id == 0)
	{
	    llenarMatriz(A);
        llenarMatriz(B);

        for (i=0; i<n; i++){
            for (j=0; j<n; j++){
                C[i][j] =0;
            }
        }

        printf("\n");

        /*Ahora basicamente lo que hacemos es enviar a cada proceso del plano cero
        una parte de A y B que es la que les corresponde, enviamos a cada uno ya con la distribuci�n
        inicial para que puedan empezar multiplicando*/
        timeIni = MPI_Wtime();
        for (i=0; i < m; i++){
		    for (j=0; j<m; j++){
                if(!(i==0 && j==0)){
                    coords_envio[0] = i;
                    coords_envio[1] = j;
                    cont_fila=-1;
                    for (k=i*tam_subM; k < i*tam_subM+tam_subM; k++){
                        cont_fila++;
                        cont_columna=0;
                        for (l=j*tam_subM; l < j*tam_subM+tam_subM; l++){
                            subm_A[cont_fila][cont_columna]=A[k][l];
                            subm_B[cont_fila][cont_columna]=B[k][l];
                            cont_columna++;
                        }
                    }
                    /*calculamos el envio para A*/
                    /*Lo que se hace es modificar la coordenada referente
                    a la columna para enviar A al Pi,j que corresponde y luego
                    en la recepcion ya puede proceder directamente a la multiplicacion*/
                    coords_envio[0] = i;
                    coords_envio[1] = j-i;
                    if(coords_envio[1] < 0){
                        coords_envio[1] = coords_envio[1] + m;
                    }
                    MPI_Cart_rank(comm2d, coords_envio, &rank_envio);
                    MPI_Send(subm_A, (tam_subM*tam_subM), MPI_FLOAT, rank_envio, 1, comm2d);

                    /*calculamos el envio para B*/
                    /*Lo que se hace es modificar la coordenada referente
                    a la fila para enviar B al Pi,j que corresponde y luego
                    en la recepcion ya puede proceder directamente a la multiplicacion*/
                    coords_envio[1] = j;
                    coords_envio[0] = i-j;
                    if(coords_envio[0] < 0){
                        coords_envio[0] = coords_envio[0] + m;
                    }
                    MPI_Cart_rank(comm2d, coords_envio, &rank_envio);
                    MPI_Send(subm_B, (tam_subM*tam_subM), MPI_FLOAT, rank_envio, 2, comm2d);
                }
		    }
		}
		/*CARGO lo QUE CORRESPONDE AL PROCESO 0*/
		for (k=0; k<tam_subM; k++){
            for (l=0;l < tam_subM; l++){
                subm_A[k][l]=A[k][l];
                subm_B[k][l]=B[k][l];
            }
        }
        /*RESUELVO LA PARTE DE LA MATRIZ QUE SE ME QUEDO
        EN EL PROCESO 0*/

        for (ciclos = 0; ciclos < tam_subM; ciclos++) {
            for (i = 0; i < tam_subM; i++) {
                for (j = 0; j < tam_subM; j++) {
                    for (k = 0;  k < tam_subM; k++) {
                        subm_C[i][j] += subm_A[i][k] * subm_B[k][j];
                    }
                }
            }
            MPI_Cart_shift(comm2d,1,-1,&fuente,&destino);
            MPI_Sendrecv_replace(subm_A,tam_subM*tam_subM,MPI_FLOAT,destino,1,fuente,1,comm2d,&statusA);

            MPI_Cart_shift(comm2d,0,-1,&fuente,&destino);
            MPI_Sendrecv_replace(subm_B,tam_subM*tam_subM,MPI_FLOAT,destino,2,fuente,2,comm2d,&statusB);
        }

	}
	else{
	    /*LOS OTROS PROCESOS RECIBEN SUS VALORES Y VAN HACIENDO SHIFT Y ENVIANDO A LOS DEMAS PARA
	    QUE CADA UNO COMPUTE SU PARTE*/

	    MPI_Recv(subm_A, tam_subM*tam_subM, MPI_FLOAT, 0, 1, comm2d, &statusA);

		MPI_Recv(subm_B,tam_subM*tam_subM,MPI_FLOAT,0,2,comm2d,&statusB);

		for (ciclos = 0; ciclos < tam_subM; ciclos++) {
            for (i = 0; i < tam_subM; i++) {
                for (j = 0; j < tam_subM; j++) {
                    for (k = 0;  k < tam_subM; k++) {
                        subm_C[i][j] += subm_A[i][k] * subm_B[k][j];
                    }
                }
            }
            MPI_Cart_shift(comm2d,1,-1,&fuente,&destino);
            MPI_Sendrecv_replace(subm_A,tam_subM*tam_subM,MPI_FLOAT,destino,1,fuente,1,comm2d,&statusA);

            MPI_Cart_shift(comm2d,0,-1,&fuente,&destino);
            MPI_Sendrecv_replace(subm_B,tam_subM*tam_subM,MPI_FLOAT,destino,2,fuente,2,comm2d,&statusB);
        }

        /*AHORA LO QUE HACEMOS ES ENVIAR AL PROCESO MAESTRO EL RESULTADO FINAL QUE OBTUVIMOS PARA NUESTRA
        SUBMATRIZ C*/
        MPI_Send(subm_C,tam_subM*tam_subM,MPI_FLOAT,0,mi_id,comm2d);
	}

	if(mi_id == 0){
	    for (i=0; i<n; i++){
            for (j=0; j<n; j++){
                C[i][j] =0;
            }
        }
	    /*RECIBIMOS Y ESTABLECEMOS CADA SUBMATRIZ C EN LA PARTE QUE CORRESPONDE*/
	    for(i=1; i < size; i++)
		{
			MPI_Recv(subm_C_aux,tam_subM*tam_subM, MPI_FLOAT,MPI_ANY_SOURCE,MPI_ANY_TAG,comm2d,&statusC);
			MPI_Cart_coords(comm2d,statusC.MPI_TAG,2, coords_envio);

			fila_recepcion = coords_envio[0];
			col_recepcion = coords_envio[1];
			cont_columna =0;
			cont_fila =-1;
			for(j=fila_recepcion*tam_subM; j<fila_recepcion*tam_subM+tam_subM; j++){
			    cont_fila++;
				for(k=col_recepcion*tam_subM; k<col_recepcion*tam_subM+tam_subM; k++){
					C[j][k]= subm_C_aux[cont_fila][cont_columna];
					cont_columna++;
				}
				cont_columna=0;
			}
		}
		/*AHORA ESTABLECEMOS LO QUE COMPUTO EL PROCESO 0*/
		for (k=0; k<tam_subM; k++){
            for (l=0;l < tam_subM; l++){
                C[k][l]=subm_C[k][l];
            }
        }
        printf("INICIAMOS IMPRESI�N A ARCHIVO...\n");
        timeFin = MPI_Wtime();
		printf("TIEMPO TARDADO---> %f segundos\n", timeFin-timeIni);
        if((fp=freopen("ResultadosCannon.txt", "w" ,stdout))==NULL) {
            printf("NO PUEDO ABRIR ARCHIVO.\n");
            exit(1);
        }
        printf("\n\nTIEMPO TARDADO---> %f segundos\n", timeFin-timeIni);
        printf("\n\nIMPRESION DE LA MATRIZ A\n.");
        imprimirMatriz(A);
        printf("\n\nIMPRESION DE LA MATRIZ B\n.");
        imprimirMatriz(B);

        printf("\n\nIMPRESION FINAL DE LA MATRIZ C\n.");
        imprimirMatriz(C);
        fclose(fp);
	}

    MPI_Finalize();
}
