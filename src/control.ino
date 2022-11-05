//ARDUINO CONTROL

//RECEPTOR SERIAL

// Detectores Capacitivos 
int i_global_pin_2B2 = 2;  // LS+201
int i_global_pin_2B3 = 3;  // LS-202
int i_global_pin_2B4 = 4;  // LS-203
int i_global_pin_2B5 = 5;  // LS-204
int i_global_pin_2B6 = 6;  // LS+205
int i_global_pin_2B7 = 7;  // LS-206

// Válvulas monoestables
int i_global_pin_2M3 = 8;    // V201
int i_global_pin_2M4 = 9;  // V202
int i_global_pin_2M5 = 10; // V203

// Bombas
int i_global_pin_2M1 = 11; // Motor bomba principal (PWM)
int i_global_pin_2M2 = 12; // Motor bomba recirculación

// Switch de puesta en marcha
int i_global_pin_S0 = 13;

// Variables
int i_global_2B1; // FIC201
int  i_global_2B2;
int  i_global_2B3;
int  i_global_2B4;
int  i_global_2B5;
int  i_global_2B6;
int  i_global_2B7;

int i_global_2M3;
int i_global_2M4;
int i_global_2M5;

int i_global_2M1;
int i_global_2M2;

int i_global_S0;

//Variables de lectura serial (seleccion de parametros)
int i_global_controlMenu=0;
int i_global_cantMezcla; //en mililitros
float f_global_voltajeBomba;//EN VOLTAJE POR AHORA, CAMBIAR DESPUES
int i_global_receta;//Variable que recibe el numero de receta (1,2 o 3)
int i_global_recirculacion; // 1 para recircular, 0 para no recircular
struct struct_receta{ //crea tipo de variable struct receta con 3 liquidos (porcentaje por mililitros de mezcla total)
   float liquido1;
   float liquido2;
   float liquido3;
} s_global_receta;

//Palabra para mandar a labview
String palabra="";

//Se crea variable PWM para controlar bomba analogica
int PWM;

//Bandera para indicar que está listo para volver a escuchar del serial
int i_bandera_serial=1;

//Declaracion de variables de comunicacion serial
char c_global_buffer[11];

//Declaracion de contadores para indicar en que proceso esta
int i_global_contador_subproceso=0; //Indica en que proceso esta para seleccionar subrutina del loop principal
int i_global_contador_controlMezcla=0;//indica en que proceso va dentro de la subrutina de controlMezcla

//Declaración e inicialización de variables tiempo para realizar el muestreo de 100ms para el control de mezcla 
int t_act=millis(), t_ant=millis();

//Variable global para calculo de suma en control de caudal
float i_global_control_suma=0; //flotante porque la suma es de ml/10ms (valores pequeños)

//********************************************

void setup()
 { 
   // Inicialización de comunicación serial
   Serial.begin(9600);
   // Detectores Capacitivos 
   pinMode(i_global_pin_2B2, INPUT);
   pinMode(i_global_pin_2B3, INPUT);
   pinMode(i_global_pin_2B4, INPUT);
   pinMode(i_global_pin_2B5, INPUT);
   pinMode(i_global_pin_2B6, INPUT);
   pinMode(i_global_pin_2B7, INPUT);
   // Válvulas monoestables
   pinMode(i_global_pin_2M3, OUTPUT);
   pinMode(i_global_pin_2M4, OUTPUT);
   pinMode(i_global_pin_2M5, OUTPUT);
   // Bombas
   pinMode(i_global_pin_2M1, OUTPUT);
   pinMode(i_global_pin_2M2, OUTPUT);
  //Salidas digitales para comunicacion en paralelo con HMI
  pinMode(A1,OUTPUT); //Error
  pinMode(A2,OUTPUT); //Menu
  pinMode(A3,OUTPUT); //Bit 0 - Estado mezcla
  pinMode(A4,OUTPUT); //bit 1 - Estado mezcla
  digitalWrite(A2,HIGH); //Empieza escuchando
  digitalWrite(A3,HIGH); //Empieza A3 prendido, como si acabara de recircular, listo para escuchar los parametros de mezcla
  digitalWrite(A4,HIGH); //Empieza A4 prendido, como si acabara de recircular, listo para escuchar los parametros de mezcla


 }

void loop()
 { 
   //Lectura de los valores actuales de los sensores y de los actuadores para actualizar la "lista de asignaciones"
   lectura();
  
  
  
  //Verifica queno haya error por estado de sensores y actuadores
  checarError();

  //Subrutina que escucha al arduino HMI en espera de ordenes
  comunicacionSerial();

  //Subrutina para leer  estado de entradas y salidas si es requerido
  //pruebasEntradas();

  recirculacion();

   
   //tiempo actual = milisegundos / 100, cada 100 milisegundos t_act aumentará en 1 unidad 
   //t_act y t_ant se utilizan en la subrutina de controlMezcla() para aumentar un contador cada vez que pasen 100 milisegundos
   t_act=millis()/100;

  //Subrutina para control de mezcla
   controlMezcla();

  //envia informacion a labview
  envioLabview();

  //Igualar tiempo anterior con tiempo actual, para comparar posteriormente en la funcion de controlMezcla
   t_ant=t_act; 

  
   
 }

//********************************************
//SUBRUTINAS Y FUNCIONES

 void lectura()
 {
   //Hace la lectura de las entradas y salidas 
   //y guarda sus estados en una lista de asignaciones
 
   //Lectura de entradas
   i_global_2B1 = analogRead(A0);
   i_global_2B2 = digitalRead(i_global_pin_2B2);
   i_global_2B3 = digitalRead(i_global_pin_2B3);
   i_global_2B4 = digitalRead(i_global_pin_2B4);
   i_global_2B5 = digitalRead(i_global_pin_2B5);
   i_global_2B6 = digitalRead(i_global_pin_2B6);
   i_global_2B7 = digitalRead(i_global_pin_2B7);
   //Lectura de salidas
   i_global_2M2 = digitalRead(i_global_pin_2M2);
   i_global_2M3 = digitalRead(i_global_pin_2M3);
   i_global_2M4 = digitalRead(i_global_pin_2M4);
   i_global_2M5 = digitalRead(i_global_pin_2M5);
   //Lectura de S0
   i_global_S0 = digitalRead(i_global_pin_S0);
 }


void envioLabview(){
    

  if(t_act!=t_ant){

    char arreglo_palabra[7];
    if(i_global_2B2==HIGH){arreglo_palabra[0]='1';}else{arreglo_palabra[0]='0';}
    if(i_global_2B3==HIGH){arreglo_palabra[1]='1';}else{arreglo_palabra[1]='0';}
    if(i_global_2B4==HIGH){arreglo_palabra[2]='1';}else{arreglo_palabra[2]='0';}
    if(i_global_2B5==HIGH){arreglo_palabra[3]='1';}else{arreglo_palabra[3]='0';}
    if(i_global_2B6==HIGH){arreglo_palabra[4]='1';}else{arreglo_palabra[4]='0';}
    if(i_global_2B7==HIGH){arreglo_palabra[5]='1';}else{arreglo_palabra[5]='0';}
    arreglo_palabra[6]=' ';
    palabra=arreglo_palabra;
    Serial.print(palabra);
    Serial.print("C");
    int caudal=((i_global_2B1*0.0596)+0.3781)*60;//ml/s
    Serial.println(caudal);
  }
}
 
 void pruebasEntradas()
 {

   //Esta subrutina imprime estado de entradas y salidas en el monitor serial
  //con el fin de informar al operador si es necesario, aunque se recomienda realizar cambios en la programacion para esto
   if(millis()%1000 == 0)
   {
      Serial.println("ESTADO DE LAS ENTRADAS:");
      Serial.print("S0 \t");
      Serial.println(i_global_S0);
      Serial.print("2B1 \t");
      Serial.println(i_global_2B1);
      Serial.print("2B2 \t");
      Serial.println( i_global_2B2);
      Serial.print("2B3 \t");
      Serial.println( i_global_2B3);
      Serial.print("2B4 \t");
      Serial.println( i_global_2B4);
      Serial.print("2B5 \t");
      Serial.println( i_global_2B5);
      Serial.print("2B6 \t");
      Serial.println( i_global_2B6);
      Serial.print("2B7 \t");
      Serial.println( i_global_2B7);
      
      Serial.print("2M1 \t");
      Serial.println( i_global_2M1);
      Serial.print("2M2 \t");
      Serial.println( i_global_2M2);
      Serial.print("2M3 \t");
      Serial.println( i_global_2M3);
      Serial.print("2M4 \t");
      Serial.println( i_global_2M4);
      Serial.print("2M5 \t");
      Serial.println( i_global_2M5);
      
      Serial.println("*************");
   }
 }
 
//Subrutina para leer informacion de la terminal serial mandada por el Arduino HMI
void comunicacionSerial(){
  //Se pregunta si hay cambios en el serial y si se está listo para escuchar otra vez
  if(Serial.available()&&i_bandera_serial){

    //Se leen de la comunicacion serial cada byte de una cadena de 11 caracteres, hasta encontrar un salto de linea
    //gurada cada byte en una posicion del arreglo de caracteres c_global_buffer, con dimension 11(incluye el salto de linea)
    Serial.readBytesUntil('\n',c_global_buffer,11);

    //Primer cifra de lectura, 0 = No empieces mezcla, 1 = empieza mezcla.
    i_global_controlMenu=(c_global_buffer[0]-48);
    //Cifras 2, 3, 4, 5: cantidad total de mezcla en valor decimal, se realiza la conversión a decimal:
    i_global_cantMezcla=(c_global_buffer[1]-48)*1000+(c_global_buffer[2]-48)*100+(c_global_buffer[3]-48)*10+(c_global_buffer[4]-48);
    //Cifras 5, 6, 7: cifras en voltaje*10, se realiza la conversion para pasarlo a valor flotante de voltaje (075 = 7.5V), para pasar a PWM posteriormente
    f_global_voltajeBomba=(c_global_buffer[5]-48)*10+(c_global_buffer[6]-48)+(c_global_buffer[7]-48)*0.1;
    //Cifra 9 de lectura, receta 1, 2 o 3
    i_global_receta=(c_global_buffer[8]-48);
    //Cifra 9 de lectura, 1 para recircular liquido (2M2), 0 para no recircular
    i_global_recirculacion=(c_global_buffer[9]-48);
    if(i_global_2B2)i_global_recirculacion=0;//asegura que si 2B2 ya esta encendido, recirculacion no empiece
    
    //Impresion para comprobar asignacion de valores (descomentar si se requiere)
    /*
    Serial.println("hay lectura:");
    Serial.print("controlMenu: ");
    Serial.println(i_global_controlMenu);
    Serial.print("Cantidad mezcla: ");
    Serial.println(i_global_cantMezcla);
    Serial.print("Voltaje Bomba: ");
    Serial.println(f_global_voltajeBomba);
    Serial.print("Receta: ");
    Serial.println(i_global_receta);
    Serial.print("Recirculacion: ");
    Serial.println(i_global_recirculacion);
    Serial.println();*/

    //La funcion de transferencia fue obtenida experimentalmente, midiendo voltaje que entra al modulo contro PWM, y realizando una regresion lineal en excel
    PWM = 25.978*(f_global_voltajeBomba) - 12.01;

    //Llamada a subrutina para calcular cantidad de liquido 1, 2 y 3, dependiendo de la seleccion
    crearReceta();

    if(!i_global_controlMenu||!i_global_recirculacion){
         digitalWrite(A2,HIGH);
         digitalWrite(A3,HIGH);
         digitalWrite(A4,HIGH);

    }
    
    //Funcion que limpia el buffer una vez que se realizó la lectura
    serialFlush();
    
    if(i_global_controlMenu||i_global_recirculacion){
      digitalWrite(A2,LOW);
      i_bandera_serial=0; //Indica que por el momento no va a escuchar nada, hasta que termine mezcla y se le de set a la bandera
    }
    
    
  
  }
}

//Subrutina que se encarga de calcular los valores para la receta seleccionada
void crearReceta(){


  switch(i_global_receta){

    case 1:
      s_global_receta.liquido1=i_global_cantMezcla*0.2;
      s_global_receta.liquido2=i_global_cantMezcla*0.3;
      s_global_receta.liquido3=i_global_cantMezcla*0.5;
      /*Serial.println("Receta 1 ");
      Serial.print("Liquido 1: ");
      Serial.println(s_global_receta.liquido1);
      Serial.print("Liquido 2: ");
      Serial.println(s_global_receta.liquido2); 
      Serial.print("Liquido 3: ");
      Serial.println(s_global_receta.liquido3);
      Serial.println();*/
      break;

    case 2:
      s_global_receta.liquido1=i_global_cantMezcla*0.3;
      s_global_receta.liquido2=i_global_cantMezcla*0.3;
      s_global_receta.liquido3=i_global_cantMezcla*0.4;
      /*Serial.println("Receta 2 ");
      Serial.print("Liquido 1: ");
      Serial.println(s_global_receta.liquido1);
      Serial.print("Liquido 2: ");
      Serial.println(s_global_receta.liquido2); 
      Serial.print("Liquido 3: ");
      Serial.println(s_global_receta.liquido3);
      Serial.println();*/
      break;

    case 3:
      s_global_receta.liquido1=i_global_cantMezcla*(0.1);
      s_global_receta.liquido2=i_global_cantMezcla*(0.4);
      s_global_receta.liquido3=i_global_cantMezcla*(0.5);
      /*Serial.println("Receta 3 ");
      Serial.print("Liquido 1: ");
      Serial.println(s_global_receta.liquido1);
      Serial.print("Liquido 2: ");
      Serial.println(s_global_receta.liquido2); 
      Serial.print("Liquido 3: ");
      Serial.println(s_global_receta.liquido3);
      Serial.println();*/
      break;

    default:
      break;
  }

}

//Esta subrutina se encarga de realizar la mezcla, dados los parámetros seleccionados por la función de selección de parámetros, por el HMI
void controlMezcla(){
 
  //Se pregunta si la bandera controlMenu está encendida o no, con el fin de que el Arduino HMI comunique al de control si debe comenzar la mezcla o no
  if(i_global_controlMenu){
    
    //Se pregunta en qué paso va del control de mezcla 
    switch(i_global_contador_controlMezcla){
     
      //Caso 0 = Paso 1 de subproceso "controlMezcla"
      // Resetea el valor de los contadores y de la suma
      case 0: 

         i_global_control_suma=0; //reseteo de la variable para que empiece en 0 la suma del caudal
         i_global_contador_controlMezcla=1; //manda a siguiente paso del control mezcla

        //Salidas para avisar a HMI en qué proceso está (Pasa a proceso deposito 1)
        digitalWrite(A3,LOW);
        digitalWrite(A4,LOW);
        
        
          
         break;
      
      //Caso 1 = Paso 2 de subproceso "controlMezcla"
      // con Valvula 1 abierta y motor 1 prendido, empieza a realizar integracion numerica del caudal para obtener cantidad de mezcla y cerrar valvula1 y apagar motor
      case 1:

        // Abre valvula 2M3
         digitalWrite(i_global_pin_2M3,HIGH);
        // Enciende motor 2M1 NOTA: PWM dependerá del voltajeBomba seleccionado y su mapeo correspondiente
        analogWrite(i_global_pin_2M1,PWM);

        //Cada vez que el reloj detecta un aumento en incrementos de 100ms, hace el calculo, ya que este es el valor de muestreo 
         if(t_act!=t_ant){

          //(i_global_2B1*0.0596)+0.3781) --> Funcion de transferencia que proviene de mediciones de caudalimetro vs lectura del convertidor A/D 
          //en las pruebas de interfaz de entrada del sensor del caudal (Archivo "Mapeo_caudalimetro-caudal.xlsx").
          //Para obtener una integracion numerica con una base de 100ms y altura en función del caudal, en mililitros
          //sumando cada iteración, obtenemos una aproximación de la cantidad total del líquido transferido a la estación de mezcla. la variable que guarda la suma es i_global_control_suma.
          // La FdeT entrega un resultado en [ml/s], por lo tanto, debido a que el muestreo es de 100ms, se divide el resultado entre 10 (=ml/100ms) y se suma.
          float i_2B2_caudal = ((i_global_2B1*0.0596)+0.3781); 
          i_global_control_suma=(i_global_control_suma)+i_2B2_caudal/10;
          
          //Para ver en monitor serial los resultados de cada iteración, descomentar lo siguiente
          /*Serial.print("Caudal instantaneo [ml/100ms]: ");
          Serial.println(i_2B2_caudal/10,5);
          Serial.print("Suma: ");
          Serial.println(i_global_control_suma,5);*/
         }
         

         if(i_global_control_suma>=s_global_receta.liquido1-((f_global_voltajeBomba-4)*10)){//si se iguala o supera, apaga valvula 1, prende 2 y cambia al siguiente proceso (contador = 2)
          /*Serial.print("Volumen total de líquido: ");
          Serial.println(i_global_control_suma); //imprime volumen final
          Serial.println(" ");*/

          digitalWrite(i_global_pin_2M3,LOW);// Cierra valvula 2M3 
          digitalWrite(i_global_pin_2M1,LOW);// apaga motor 2M1

          Serial.print(palabra);
          Serial.print("C");
          Serial.println("0");

          i_global_control_suma=0;//reinicia las variables para que se reutilicen en el siguiente ciclo
          i_global_contador_controlMezcla=2;
          delay(3000);
          //Salidas para avisar a HMI en qué proceso está (Pasa a proceso deposito 1)
          digitalWrite(A3,LOW);
          digitalWrite(A4,HIGH);
          
          
         }

         break;
        
      //Caso 2 = Paso 3 de subproceso "controlMezcla"
      // con Valvula 2 abierta y motor 1 prendido, empieza a realizar integracion numerica del caudal para obtener cantidad de mezcla y cerrar valvula2 y apagar motor
      case 2:

        // Abre valvula 2M4 
               digitalWrite(i_global_pin_2M4,HIGH);
              // Enciende motor 2M1
               analogWrite(i_global_pin_2M1,PWM);

         if(t_act!=t_ant){//muestrea la variable y hace la suma cada 100 ms

          float i_2B2_caudal = ((i_global_2B1*0.0596)+0.3781); 
          i_global_control_suma=(i_global_control_suma)+i_2B2_caudal/10;

          //Para ver en monitor serial los resultados de cada iteración, descomentar lo siguiente
          /*Serial.print("Caudal instantaneo [ml/100ms]: ");
          Serial.println(i_2B2_caudal/10,5);
          Serial.print("Suma: ");
          Serial.println(i_global_control_suma,5);*/

         }
    
         if(i_global_control_suma>=s_global_receta.liquido2-((f_global_voltajeBomba-4)*10)){//si se supera, apaga valvula 3, prende 3 y cambia al siguiente proceso (contador = 3)
          /*Serial.print("Volumen total de líquido: ");
          Serial.println(i_global_control_suma); //imprime volumen final
          Serial.println(" ");*/

          digitalWrite(i_global_pin_2M4,LOW);// Cierra valvula 2M3 
          digitalWrite(i_global_pin_2M1,LOW);// apaga motor 2M1

          Serial.print(palabra);
          Serial.print("C");
          Serial.println("0");
          
          i_global_control_suma=0;//reinicia las variables para que se reutilicen en el siguiente ciclo
          i_global_contador_controlMezcla=3;
          delay(3000);
          //Salidas para avisar a HMI en qué proceso está (Pasa a proceso deposito 1)
          digitalWrite(A3,HIGH);
          digitalWrite(A4,LOW);
          
          
         }

         break;

      //Caso 3 = Paso 4, ultimo paso de subproceso "controlMezcla"
      // con Valvula 3 abierta y motor 1 prendido, empieza a realizar integracion numerica del caudal para obtener cantidad de mezcla y cerrar valvula3 y apagar motor y finalizar
      case 3:

          // Abre valvula 2M5 
         digitalWrite(i_global_pin_2M5,HIGH);
        // Enciende motor 2M1
         analogWrite(i_global_pin_2M1,PWM);

         if(t_act!=t_ant){//muestrea la variable y hace la suma cada 100 ms

          float i_2B2_caudal = ((i_global_2B1*0.0596)+0.3781); 
          i_global_control_suma=(i_global_control_suma)+i_2B2_caudal/10;

          //Para ver en monitor serial los resultados de cada iteración, descomentar lo siguiente
          /*Serial.print("Caudal instantaneo [ml/100ms]: ");
          Serial.println(i_2B2_caudal/10,5);
          Serial.print("Suma: ");
          Serial.println(i_global_control_suma,5);*/

         }
         
         if(i_global_control_suma>=s_global_receta.liquido3-((f_global_voltajeBomba-4)*10)){//si se supera, apaga valvula 3, prende 3 y cambia al siguiente proceso (contador = 3)
          /*Serial.print("Volumen total de líquido: ");
          Serial.println(i_global_control_suma); //imprime volumen final
          Serial.println(" ");*/

          digitalWrite(i_global_pin_2M5,LOW);// Cierra valvula 2M5 
          digitalWrite(i_global_pin_2M1,LOW);// Apaga motor 2M1

          Serial.print(palabra);
          Serial.print("C");
          Serial.println("0");

          i_global_control_suma=0;//reinicia las variables para que se reutilicen en el siguiente ciclo
          i_global_contador_controlMezcla=0;// reinicia variable para empezar en 0 si se vuelve a comenzar una mezcla
          i_global_controlMenu=0;//Reinicia bandera para poder volver a iniciar el proceso de mezcla si se pide
          serialFlush(); //si habia informacion en el buffer, la borra
          //Enciende A2 para indicar que termino la mezcla y está esperando de nuevo informacion
          digitalWrite(A2,HIGH);
          i_bandera_serial=1; //indica que se puede volver a leer del serial
          delay(3000);
         }

         break;
     
      default:
        break;

    }
  }
}

//Subrutina para recircular el liquido de la mezcla a los depositos
void recirculacion(){
  if(i_global_recirculacion&&!i_global_controlMenu&&!i_global_2B2){
    digitalWrite(i_global_pin_2M2,HIGH);
    //Salidas para avisar a HMI en qué proceso está
    digitalWrite(A3,HIGH);
    digitalWrite(A4,HIGH);
  }
}

//Comprueba que el estado de las entradas y salidas sean válidos
 void checarError(){
    
     int banderaError = 0;

     //  1. Si está encendido el motor 2M1 y cerradas todas las válvulas
     if (i_global_2M1 && (!i_global_2M3 && !i_global_2M4 && !i_global_2M5))   
     {
      banderaError = 1;
      Serial.println("ERROR1");
     }
     //  2. Si está encendido el sensor de nivel superior y está apagado el inferior, para el depósito 1
     else if (i_global_2B2 && !i_global_2B3) 
     {
      banderaError = 1;
      Serial.println("ERROR2");
     }
     // 3. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if (i_global_2B6 && !i_global_2B7) 
     {
      banderaError = 1;
      Serial.println("ERROR3");
     }
     // 4. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if ( i_global_2M3 && !i_global_2B3)
     {
      banderaError = 1;
      Serial.println("ERROR4");
     }
  // 5. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if (i_global_2M4 && !i_global_2B4)
     {
      banderaError = 1;
      Serial.println("ERROR5");
     }
  // 6. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if ( i_global_2M5 && !i_global_2B5)
     {
      banderaError = 1;
      Serial.println("ERROR6");
     }
  // 7. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if (i_global_2M2 && !i_global_2B7)
     {
      banderaError = 1;
      Serial.println("ERROR7");
     }
  // 8. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if ( (i_global_2M3 || i_global_2M4 || i_global_2M5) && i_global_2B6)
     {
      banderaError = 1;
      Serial.println("ERROR8");
     }
  // 9. Si está encendido el sensor de nivel superior y está apagado el inferior, para el tanque de mezcla
     else if (i_global_2M2 && i_global_2B2)
     {
      if(!i_global_recirculacion){
        banderaError = 1;
        Serial.println("ERROR9");
      }
      digitalWrite(i_global_pin_2M2,LOW);//Deja de recircular
      i_global_recirculacion=0;//reinicia bandera, (esta fue activada por ordenes del HMI)
      i_bandera_serial=1;//Vuelve a escuchar al serial
      digitalWrite(A2,HIGH);//Vuelve a mandar al HMI que está en espera de leer algo, al terminar la recirculacion
      
     }
     else
     {
      banderaError = 0;
     }
     
     if (banderaError == 1)
     {
      // Si hay error apaga todas las válvulas y motores, y enciende A1 para que el HMI sepa que hay error
      // Apaga los motores
      digitalWrite(i_global_pin_2M1, LOW);
      digitalWrite(i_global_pin_2M2, LOW);
      // Cierra las válvulas
      digitalWrite(i_global_pin_2M3, LOW);
      digitalWrite(i_global_pin_2M4, LOW);
      digitalWrite(i_global_pin_2M5, LOW);
      
      

      i_global_control_suma=0;//reinicia las variables para que se reutilicen en el siguiente ciclo
      i_global_contador_controlMezcla=0;// reinicia variable para empezar en 0 si se vuelve a comenzar una mezcla
      i_global_controlMenu=0;//Reinicia bandera para poder volver a iniciar el proceso de mezcla si se pide
      serialFlush(); //si habia informacion en el buffer, la borra
      i_bandera_serial=1; //indica que se puede volver a leer del serial

      // Imprime el estado de error
      digitalWrite(A1, HIGH);//Manda error al HMI
      Serial.println("ERROR!!!");
      delay(500);
      
      digitalWrite(A1,LOW);
      digitalWrite(A2,HIGH);
      digitalWrite(A3,HIGH);
      digitalWrite(A4,HIGH);
       
     }else{
      digitalWrite(A1, LOW);//Manda error al HMI
    }

 }
 

//Eliminar valores del buffer
void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}
