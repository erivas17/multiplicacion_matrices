
	/******README DE LA IMPLEMENTACION DEL ALGORITMO DE CANNON******/


	Existen dos versiones de Cannon en este cd(CANNON-MPI-IMP y CANNON-MPI-PR).

	CANNON-MPI-PR es la que se uso para hacer pruebas con grandes valores donde no 
	se envian los valores de las submatrices desde el proceso 0 y no se reciben
	de vuelta los resultados en el proceso 0, sino que cada proceso crea su submatriz
	y realiza la ejecuci�n del Algoritmo de multiplicaci�n en si, dejando de lado
	la transferencia de los resultados al proceso 0.

	CANNON-MPI-IMP es la que nos muestra la impresi�n de los resultados para demostrar
	claramente que el Algoritmio multiplica correctamente, aqui el proceso 0 distribuye
	los valores a las submatrices y recoge los resultados finales, esto para matrices muy
	grandes no funcionaria correctamente porque el proceso 0 ser�a un cuello de botella cr�tico. Es
	el mismo algoritmo que el anterior, solo que se envian valores desde el proceso 0 y se recogen
	al mismo resultados.
