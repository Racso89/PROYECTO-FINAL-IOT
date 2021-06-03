Algoritmo ESP32_FIREBASE_THINGSPEAK
	Leer TEMPERATURA
	Leer PUERTA
	Leer RELE
	Leer X
	Si CONEXION Entonces
		Escribir JSON
		Si TEMPERATURA>X Entonces
			Escribir ENVIO,CORREO
			Escribir VENTILADOR,ON
		SiNo
			Si ITERAR==TRUE Entonces
				Escribir FIREBASE
				Escribir ITERAR,FALSE
			SiNo
				Escribir MENSAJE,THINGSPEAK
			FinSi
			Si TEMPERATURA<X Entonces
				Escribir VENTILADOR,OFF
			SiNo
				Escribir PUBLICACIONGENERAL
				Escribir PUBTEM
				Escribir PUBHUM
				Si LLUVIA<700 Entonces
					Escribir ENVIA,CORREO
					Escribir SERVO,90
					Escribir ABRE,TAPA
				SiNo
					Si LLUVIA>700 Entonces
						Escribir SERVO,0
						Escribir CIERRA,TAPA
						Si SUELO<25 Y BANDERA=1 Entonces
							Escribir ENCENDER,BOMBA,AGUA,(LED)
						SiNo
							Si SUELO<25 Y BANDERA=0 Entonces
								Escribir APAGAR,BOMBA,AGUA,(LED)
							SiNo
								Escribir PUBLICACIONGENERAL
							FinSi
						FinSi
					SiNo
						Escribir PUBLICACIONGENERAL
						Leer X
						Si X>20 Entonces
							Escribir CONFIGURACION,TEMPERATURA
						SiNo
							Escribir PUBLICACIONGENERAL
						FinSi
					FinSi
				FinSi
			FinSi
		FinSi
	SiNo
		Escribir loop
		Escribir FUNCIONES
		Escribir CORREOVENTILADOR
		Escribir CORREOLLUVIA
		Escribir CALLBACK
		Escribir WIFI
		Escribir RECONNECT
	FinSi
FinAlgoritmo
