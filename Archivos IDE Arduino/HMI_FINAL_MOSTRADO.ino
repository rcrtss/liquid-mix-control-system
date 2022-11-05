#include <LiquidCrystal.h>

int contadorBoton = 0;

// PINES DEL LCD
const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;

// Se crea una variable de tipo LiquidCrystal
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//  Pines de botones
int pin_botonMas = 6; 
int pin_botonMenos = 5;
int pin_botonEnter = 4;
int pin_botonReturn = 3;

// Lectura de los botones
int lectura_botonMas = 0;
int lectura_botonMenos = 0;
int lectura_botonEnter = 0;
int lectura_botonReturn = 0;

//  Lectura anterior de los botones
int anterior_botonMas = 0;
int anterior_botonMenos = 0;
int anterior_botonEnter = 0;
int anterior_botonReturn = 0;

// Estado real de los botones (empleando antirebote por software)
int estado_botonMas = 0;
int estado_botonMenos = 0;
int estado_botonEnter = 0;
int estado_botonReturn = 0;

// Variables que registran un pulso en el boton (evita sobre lecturas por enclavamiento)
boolean pulso_botonMas = false;
boolean pulso_botonMenos = false;
boolean pulso_botonEnter = false;
boolean pulso_botonReturn = false;

// Tiempo de ultimo flanco de los botones
unsigned long lastDebounceTime_botonMas = 0;
unsigned long lastDebounceTime_botonMenos = 0;
unsigned long lastDebounceTime_botonEnter = 0;
unsigned long lastDebounceTime_botonReturn = 0;

// Tiempo de antirebote
int debounceDelay = 50;

// Inicio del hold time
unsigned long inicioHold = 0;

int holdTime = 50;  // el tiempo que tiene que pasar para volver a registrar un flanco

// Bandera de error (combinaciones de los sensores)
boolean banderaError = false;

// Bandera de menu selección
boolean banderaMenu = false;

// Contador de menu
int contadorMenu = 0;

// Banderas para borrar pantallas
boolean anterior_vaciadoDep1 = false;
boolean anterior_vaciadoDep2 = false;
boolean anterior_vaciadoDep3 = false;
boolean anterior_vaciadoTanque = false;

boolean actual_vaciadoDep1 = false;
boolean actual_vaciadoDep2 = false;
boolean actual_vaciadoDep3 = false;
boolean actual_vaciadoTanque = false;

boolean anterior_menuSeleccion = false;
boolean anterior_menuRecircula = false;

boolean actual_menuSeleccion = false;
boolean actual_menuRecircula = false;


boolean anterior_mensajeError = false;
boolean actual_mensajeError = false;

// Entradas
byte pin1, pin2, pin3, pin4;

//Variables para configuracion de mezcla. Se selecciona su valor en el menú
int cantMezcla=1500; // En mililitros
int voltajeBomba=70; // EN VOLTAJE(min 5 , max 10)
int receta = 1; // Indica la receta a preparar (3 recetas distintas)
boolean banderaIniciaMezcla = false;
boolean banderaRecircula = false;


void setup() {
  // INICIALIZAR LCD
   lcd.begin(16, 2); // (columna, renglones)
   Serial.begin(9600); // Inicia comunicacion serial a 9600 baudios
   // Asignar botones como entradas
   pinMode(pin_botonMas, INPUT);
   pinMode(pin_botonMenos, INPUT);
   pinMode(pin_botonEnter, INPUT);
   pinMode(pin_botonReturn, INPUT);
   // Asignar los pines que reciben comunicación paralela
   pinMode(A1, INPUT); // Recibe error (1)
   pinMode(A2, INPUT); // Recibe si está en menú (1)
   pinMode(A3, INPUT); // Parte del proceso de mezcla actual, combinaciones con A4
   pinMode(A4, INPUT); // Parte del proceso de mezcla actual, combinaciones con A3
 }

void loop() {
  // Debug del código por monitor serial
  /*if(millis()%1000 == 0) 
  {
      Serial.print("Contador de Menu: ");
      Serial.println(contadorMenu);
      Serial.print("Cantidad Mezcla: ");
      Serial.println(cantMezcla);
      Serial.print("Voltaje Bomba: ");
      Serial.println(voltajeBomba);
      Serial.print("Receta: ");
      Serial.println(receta);
      Serial.println("***********************");
  }*/
  
  // Lectura de entradas
  pin1 = digitalRead(A1);
  pin2 = digitalRead(A2);
  pin3 = digitalRead(A3);
  pin4 = digitalRead(A4);
  
  digitalWrite(2, pin2); // Indica si está activado el menu
  digitalWrite(13, pin1); // Indica error
  
  actual_menuSeleccion = pin2;
  if (anterior_menuSeleccion != actual_menuSeleccion)
   {
      anterior_menuSeleccion = actual_menuSeleccion;
      lcd.clear();
   }
   
   actual_mensajeError = pin1;
    if (anterior_mensajeError != actual_mensajeError)
    {
      anterior_mensajeError = actual_mensajeError;
      lcd.clear();
    }
    
    actual_vaciadoTanque = pin4;
    if (anterior_vaciadoTanque != actual_vaciadoTanque)
    {
      anterior_vaciadoTanque = actual_vaciadoTanque;
      lcd.clear();
    }
    
    

  // Lectura de botones
  lecturaBotones();

    // Checa la bandera de error
  if (pin1 ==  HIGH) banderaError = true;
  
  if (banderaError) // Si hay error
  {   
    // Imprime mensaje de error
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    if (pulso_botonEnter == HIGH) 
    {
      banderaError = false; // Si el operador da clic en enter se quita la bandera de error
      contadorMenu = 0; // Reinicia el contador de menu
      // Limpia la pantalla del LCD
      lcd.clear();
    }

  }
  else // Si no hay error
  {
    digitalWrite(13, LOW);
    // Checar si está activo el menu
    if (pin2 == HIGH) 
    {
        digitalWrite(2, HIGH);
        if (pin4 == HIGH) // Inicia con el menu de selección
        {
      switch (contadorMenu)
       {
        case 0: // Inicio
          lcd.setCursor(0, 0);          
          lcd.print("INICIO");
          if (pulso_botonEnter == HIGH)
          {
            contadorMenu++;
            // Limpia la pantalla del LCD
            lcd.clear();
          }
        break;

        case 1: // Volumen Mezcla
          lcd.setCursor(0, 0);
          lcd.print("Volumen Mezcla");
          lcd.setCursor(0, 1);
          lcd.print(cantMezcla);
          // Checar botones
          if (pulso_botonReturn == HIGH) // RETURN
          {
            contadorMenu--;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonEnter == HIGH) // ENTER
          {
            contadorMenu++;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonMas == HIGH) // Boton Mas
          {
            if (cantMezcla < 2000) cantMezcla += 20;
            lcd.clear();
          }

          else if (pulso_botonMenos == HIGH) // Boton Menos
          {
            if (cantMezcla > 500) cantMezcla -= 20;
            lcd.clear();
          }
        break;

        case 2: // Voltaje Bomba
          lcd.setCursor(0, 0);
          lcd.print("Voltaje Bomba");
          lcd.setCursor(0, 1);
          lcd.print(voltajeBomba/10.0);
          // Checar botones
          if (pulso_botonReturn == HIGH) // RETURN
          {
            contadorMenu--;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonEnter == HIGH) // ENTER
          {
            contadorMenu++;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonMas == HIGH) // Boton Mas
          {
            if(voltajeBomba < 100) voltajeBomba += 2;
            lcd.clear();
          }

          else if (pulso_botonMenos == HIGH) // Boton Menos
          {
            if (voltajeBomba > 50) voltajeBomba -= 2;
            lcd.clear();
          }              
        break;

        case 3: // Receta
          lcd.setCursor(0, 0);
          lcd.print("Receta");
          lcd.setCursor(0, 1);
          lcd.print(receta);
          // Checar botones
          if (pulso_botonReturn == HIGH) // RETURN
          {
            contadorMenu--;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonEnter == HIGH) // ENTER
          {
            contadorMenu++;
            // Limpia la pantalla del LCD
            lcd.clear();
          }

          else if (pulso_botonMas == HIGH) // Boton Mas
          {
            if (receta < 3) receta ++;
            lcd.clear();
          }

          else if (pulso_botonMenos == HIGH) // Boton Menos
          {
            if (receta > 1) receta--;
            lcd.clear();
          } 
        break;

        case 4: // Comenzar?
          lcd.setCursor(0, 0);
          lcd.print("Comenzar?");
          // Checar botones
          if (pulso_botonEnter == HIGH) // ENTER
          {
               // Limpia la pantalla del LCD
               lcd.clear();
               banderaIniciaMezcla= true;
               // Envia la información por comunicación serial al Arduino de control
               enviaParam();
               //delay(3000); // Para que me de tiempo de mover los switches
               
               // Reinicializa la variable de contador
               contadorMenu = 0;
          }
        break;
        }
     }
     else // Pregunta si se recircula o no
     {
         banderaIniciaMezcla = false;
         lcd.setCursor(0,0);
         lcd.print("Recircular?");
         if (pulso_botonEnter == HIGH) // ENTER --> Recircular
         {
      banderaRecircula = true;
      enviaParam();
      banderaRecircula = false;
      // Limpia la pantalla del LCD
      lcd.clear();
    
         }
         else if (pulso_botonReturn == HIGH) // RETURN --> No recircular
         {
      banderaRecircula = false;
      enviaParam();
      // Limpia la pantalla del LCD
      lcd.clear();
         }
         // Envia la información por comunicación serial al Arduino de control
         }
      }
      
         else  // Checa en qué parte del proceso de mezcla se encuentra
         {
         actual_menuRecircula = false;
         actual_menuSeleccion = false;
         
         if (pin3 == LOW && pin4 == LOW)  // Vaciado depósito 1
         {
      actual_vaciadoDep1 = true;
      if (anterior_vaciadoDep1 != actual_vaciadoDep1)
      {
         anterior_vaciadoDep1 = actual_vaciadoDep1;
         lcd.clear();
      }
      lcd.setCursor(0, 0);
      lcd.print("Vaciando dep 1");
         }  
         else if (pin3 == LOW && pin4 == HIGH)   // Vaciado depósito 2
         {
      if (anterior_vaciadoDep2 != actual_vaciadoDep2)
      {
         actual_vaciadoDep2 = true;
         anterior_vaciadoDep2 = actual_vaciadoDep2;
         lcd.clear();
      }
      lcd.setCursor(0, 0);
      lcd.print("Vaciando dep 2");
         }
         else if (pin3 == HIGH && pin4 == LOW)   // Vaciado depósito 3
         {
      actual_vaciadoDep3 = true;
      if (anterior_vaciadoDep3 != actual_vaciadoDep3)
      {
         anterior_vaciadoDep3 = actual_vaciadoDep3;
         lcd.clear();
      }
      lcd.setCursor(0, 0);
      lcd.print("Vaciando dep 3");
         }
         else if (pin3 == HIGH && pin4 == HIGH)   // Recirculación
         {
      lcd.setCursor(0, 0);
      lcd.print("Recirculacion...");
         }
         }
    }
}

  // ************** LECTURA BOTONES **************
void lecturaBotones()
{
  // Lee los pines de los botones
  lectura_botonMas = digitalRead(pin_botonMas);
  lectura_botonMenos = digitalRead(pin_botonMenos);
  lectura_botonEnter = digitalRead(pin_botonEnter);
  lectura_botonReturn = digitalRead(pin_botonReturn);

  // Algoritmo antirebote para botonMas
  pulso_botonMas = LOW;
  if(lectura_botonMas != anterior_botonMas) // si hay cambio de estado en la lectura
  { 
    lastDebounceTime_botonMas = millis(); // almacena el tiempo en que ocurrio
  }
  if(millis() - lastDebounceTime_botonMas > debounceDelay) // si se ha mantenido presionado por mas de 50 ms
  {  
    if(lectura_botonMas != estado_botonMas) // se compara con el estado guardado anteriormente
    { 
      estado_botonMas = lectura_botonMas; // Si es distinto el estado, se acutaliza
       if(estado_botonMas == HIGH)
       {
          inicioHold = millis();
          pulso_botonMas = HIGH;
       }
    }
  }
   if (millis() - inicioHold >holdTime)
   {
      anterior_botonMas = lectura_botonMas;
   }

  // Algoritmo antirebote para botonMenos
  pulso_botonMenos = LOW;
  if(lectura_botonMenos != anterior_botonMenos) // si hay cambio de estado en la lectura
  { 
    lastDebounceTime_botonMenos = millis(); // almacena el tiempo en que ocurrio
  }
  if(millis() - lastDebounceTime_botonMenos > debounceDelay) // si se ha mantenido presionado por mas de 50 ms
  {  
    if(lectura_botonMenos != estado_botonMenos) // se compara con el estado guardado anteriormente
    { 
      estado_botonMenos = lectura_botonMenos; // Si es distinto el estado, se acutaliza
       if(estado_botonMenos == HIGH)
       {
          inicioHold = millis();
          pulso_botonMenos = HIGH;
       }
    }
  }
   if (millis() - inicioHold >holdTime)
   {
      anterior_botonMenos = lectura_botonMenos;
   }

  // Algoritmo antirebote para boton Enter
  pulso_botonEnter = LOW;
  if(lectura_botonEnter != anterior_botonEnter) // si hay cambio de estado en la lectura
  { 
    lastDebounceTime_botonEnter = millis(); // almacena el tiempo en que ocurrio
  }
  if(millis() - lastDebounceTime_botonEnter > debounceDelay) // si se ha mantenido presionado por mas de 50 ms
  {  
    if(lectura_botonEnter != estado_botonEnter) // se compara con el estado guardado anteriormente
    { 
      estado_botonEnter = lectura_botonEnter; // Si es distinto el estado, se acutaliza
       if(estado_botonEnter == HIGH)
       {
          inicioHold = millis();
          pulso_botonEnter = HIGH;
       }
    }
  }
   if (millis() - inicioHold >holdTime)
   {
      anterior_botonEnter = lectura_botonEnter;
   }

  // Algoritmo antirebote para boton Return
  pulso_botonReturn = LOW;
  if(lectura_botonReturn != anterior_botonReturn) // si hay cambio de estado en la lectura
  { 
    lastDebounceTime_botonReturn = millis(); // almacena el tiempo en que ocurrio
  }
  if(millis() - lastDebounceTime_botonReturn > debounceDelay) // si se ha mantenido presionado por mas de 50 ms
  {  
    if(lectura_botonReturn != estado_botonReturn) // se compara con el estado guardado anteriormente
    { 
      estado_botonReturn = lectura_botonReturn; // Si es distinto el estado, se acutaliza
       if(estado_botonReturn == HIGH)
       {
          inicioHold = millis();
           pulso_botonReturn = HIGH;
       }
    }
  }
   if (millis() - inicioHold >holdTime)
   {
      anterior_botonReturn = lectura_botonReturn;
   }
}

// ****************   ENVIA PARAMETROS *********************
/*//Variables para configuracion de mezcla. Se selecciona su valor en el menú

int cantMezcla=1500; // En mililitros
int voltajeBomba=70; // EN VOLTAJE(min 50 , max 100)
int receta = 1; // Indica la receta a preparar (3 recetas distintas)
boolean banderaIniciaMezcla = false;
boolean banderaRecircula = false;*/

void enviaParam()
{
   String palabra = "";
   char iniciaMezcla;
   char cantidadMezcla[4];
   char voltBomb[3];
   char valReceta;
   char recircular;
      
   iniciaMezcla = (banderaIniciaMezcla)+48;
   cantidadMezcla[0] = (cantMezcla/1000)+48;
   cantidadMezcla[1] =  ((cantMezcla%1000)/100) +48;
   cantidadMezcla[2] = ((cantMezcla%100)/10) +48;
   cantidadMezcla[3] = (cantMezcla%10) +48;
   voltBomb[0] =(voltajeBomba / 100) +48;
   voltBomb[1] =  ((voltajeBomba%100) / 10) +48;
   voltBomb[2] = (voltajeBomba%10) +48;
   valReceta = (receta) +48;
   recircular = (banderaRecircula) +48;
   
   palabra += iniciaMezcla;
   for (int i = 0; i < 4; i++)
   {
      palabra += cantidadMezcla[i];
   }
   
   for (int i = 0; i < 3; i++)
   {
      palabra += voltBomb[i];
   }
   palabra += valReceta;
   palabra += recircular;
  
   //Serial.println("inicia mezcla(1) cantMezcla(4)   voltaje(3)    Receta(1) Recicla(1)");
   Serial.println(palabra);
   
   //lcd.clear();
   /*
   // Debug del código por monitor serial
      Serial.print("Bandera Iniinia Mezcla: ");
      Serial.println(iniciaMezcla);
      Serial.print("Cantidad Mezcla: ");
      Serial.print(cantidadMezcla[0]);
      Serial.print(cantidadMezcla[1]);
      Serial.print(cantidadMezcla[2]);
      Serial.println(cantidadMezcla[3]);
      Serial.print("Voltaje Bomba: ");
      Serial.print(voltBomb[0]);
      Serial.print(voltBomb[1]);
      Serial.println(voltBomb[2]);
      Serial.print("Receta: ");
      Serial.println(valReceta);
      Serial.print("Recircular? ");
      Serial.println(recircular);
      Serial.println("***********************");
  */
}
