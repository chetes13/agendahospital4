#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>     // Manejo de tiempos
#include <limits>     // Necesario para std::numeric_limits
#include <iomanip>    // Agrega esta linea para std::get_time
#include <sstream>    // Agrega esta linea para std::istringstream
#include <fstream>    // Tratamiento de archivos
#include <cstdlib>    // Necesario para std::rand() y std::srand()

// ~~~~~~~ Structures
struct Cita {
  std::string nombrePaciente;
  std::chrono::system_clock::time_point fechaHora;
};

struct Doctor {
  std::string nombre;
  std::vector<int> diasTrabajo; // del 1 (lunes) al 7 (domingo)
  int horaEntrada;
  int horaSalida;
  std::vector<Cita> citas;
  int idDoctor;
};

struct Clinica {
  std::string nombre;
  std::vector<Doctor> doctores;
};

std::vector<Clinica> clinicas;
std::vector<Doctor> doctores;

std::vector<std::string> nombresDoctores;
std::vector<int> idsDoctores;


// ~~~~~~~ Funciones de ayuda
// Funcion para pedir numeros al usuario
double pedir_numero() {
  double numero{ 0 };
  do {
    if (std::cin >> numero && numero != 0 && std::cin.peek() == '\n') {
      return numero;
    }
    else {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Ingresa solamente un numero valido por favor." << std::endl;
    }
  } while (true);
}

// Crea un id unico para cada doctor
int creaIdDoctor() {
  while (true) {
    // Configura la semilla inicial para std::rand() (generalmente se usa el tiempo actual)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Genera un numero aleatorio entre 1 y 100000
    int id = std::rand() % 100000 + 1;

    auto iterador = std::find(idsDoctores.begin(), idsDoctores.end(), id);

    if (iterador == idsDoctores.end()) {
      // El numero no esta en el vector, devuelve el valor unico
      idsDoctores.push_back(id);
      return id;
    }
  }
}

// Separa x lineas en consola
void separador(int lineas = 3) {
  for (size_t i = 0; i < lineas; i++)
  {
    std::cout << "\n" << std::endl;
  }
}

// Busca el nombre del doctor en el array de nombesDoctores
bool busca_nombre_doctor(std::string nombreDoctor) {
  auto iterador = std::find(nombresDoctores.begin(), nombresDoctores.end(), nombreDoctor);

  if (iterador == nombresDoctores.end()) {
    // No lo encontro, lo guarda y regresa true
    return false;
  }

  return true;
}

// formatea la fecha y hora de las citas para mostrarlas en consola
std::string formatearFechaHora(const std::chrono::system_clock::time_point& fechaHora) {
  std::time_t tiempoT = std::chrono::system_clock::to_time_t(fechaHora);
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M", std::localtime(&tiempoT));
  return buffer;
}

void eliminarNombreDoctor(const std::string& nombre) {
  auto nuevoFin = std::remove(nombresDoctores.begin(), nombresDoctores.end(), nombre);
  nombresDoctores.erase(nuevoFin, nombresDoctores.end());
}


// ~~~~~ validaciones
// Valida si la hora es valida
bool esHoraValida(int hora) {
  return hora >= 0 && hora < 24;
}

// Valida los dias de trabajo de un doctor
bool validaDiasTrabajo(int diaInicio, int diaFin) {
  if (diaInicio < 1 || diaInicio > 7 || diaFin < 1 || diaFin > 7 || diaInicio > diaFin) {
    return false;
  }
  else {
    return true;
  }
}

// chequeo de compitaibilidad horario doctor-cita
bool esHorarioValidoParaDoctor(const Doctor& doctor, const std::chrono::system_clock::time_point& fechaHora) {
  auto diaSemanaCita = std::chrono::system_clock::to_time_t(fechaHora);
  struct tm* partesTiempo = std::localtime(&diaSemanaCita);

  int diaSemana = partesTiempo->tm_wday; // Los dias de la semana van de 0 (domingo) a 6 (sabado)
  int horaCita = partesTiempo->tm_hour;

  // Convertir el dia de la semana a un formato compatible con nuestro sistema (1 = lunes, 7 = domingo)
  diaSemana = diaSemana == 0 ? 7 : diaSemana;

  // Verificar si el dia de la semana esta dentro de los dias de trabajo del doctor
  if (std::find(doctor.diasTrabajo.begin(), doctor.diasTrabajo.end(), diaSemana) == doctor.diasTrabajo.end()) {
    return false;
  }

  // Obtener la hora de entrada y salida del doctor en formato de 24 horas
  int horaEntrada = doctor.horaEntrada;

  int horaSalida = doctor.horaSalida;

  // Verificar si la hora de la cita esta dentro del horario de trabajo del doctor
  return horaCita >= horaEntrada && horaCita <= horaSalida;
}

// chequeo de disponibilidad para agendar una cita
bool estaDisponibleElDoctor(const Doctor& doctor, const std::chrono::system_clock::time_point& fechaHora) {
  for (const auto& cita : doctor.citas) {
    if (cita.fechaHora == fechaHora) {
      return false;
    }
  }
  return true;
}

// chequeo de disponibilidad para agendar una cita editar excluyendo el indice de la cita editando
bool estaDisponibleElDoctorEditar(const Doctor& doctor, const std::chrono::system_clock::time_point& fechaHora, size_t indiceCitaExcluido) {
  for (size_t i = 0; i < doctor.citas.size(); ++i) {
    if (i != indiceCitaExcluido && doctor.citas[i].fechaHora == fechaHora) {
      return false;
    }
  }
  return true;
}

// ======= Funciones de guardado y cargado =======

std::chrono::system_clock::time_point parseFechaHora(const std::string& fechaHoraStr) {
  std::tm tm = {};
  std::istringstream ss(fechaHoraStr);
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void guardarDatos() {
  std::ofstream archivo("datos_clinicas.txt");

  if (!archivo.is_open()) {
    std::cerr << "Error al abrir el archivo para guardar los datos." << std::endl;
    return;
  }

  for (const auto& clinica : clinicas) {
    archivo << "Clinica:" << clinica.nombre << "\n";
    for (const auto& doctor : clinica.doctores) {
      archivo << "Doctor:" << doctor.nombre << "\n";
      archivo << "ID:" << doctor.idDoctor << "\n";
      archivo << "Horario:" << doctor.horaEntrada << "-" << doctor.horaSalida << "\n";
      archivo << "Dias de Trabajo:";
      for (const auto& dia : doctor.diasTrabajo) {
        archivo << dia << " ";
      }
      archivo << "\nCitas:\n";
      for (const auto& cita : doctor.citas) {
        std::time_t tiempoT = std::chrono::system_clock::to_time_t(cita.fechaHora);
        archivo << "Cita:" << std::put_time(std::localtime(&tiempoT), "%Y-%m-%d %H:%M") << ",Paciente:" << cita.nombrePaciente << "\n";
      }
    }
    archivo << "\n";  // Espacio entre clínicas
  }

  archivo.close();
}



void cargarDatos() {
  std::ifstream archivo("datos_clinicas.txt");
  if (!archivo.is_open()) {
    std::cerr << "Error al abrir el archivo para leer los datos." << std::endl;
    return;
  }

  std::string linea;
  Clinica clinicaActual;
  Doctor doctorActual;
  Cita citaActual;
  bool leyendoCitas = false;

  while (std::getline(archivo, linea)) {

    std::istringstream ss(linea);
    std::string nombre;
    std::string tipo, valor;

    std::getline(ss, tipo, ':');

    std::cout << "tipo // " << tipo << std::endl;

    if (tipo == "Clinica") {
      if (!doctorActual.nombre.empty()) {
        clinicaActual.doctores.push_back(doctorActual);
        doctorActual = Doctor();
      }
      if (!clinicaActual.nombre.empty()) {
        clinicas.push_back(clinicaActual);
        clinicaActual = Clinica();
      }

      std::getline(ss, valor);
      std::cout << "valor // " << valor << std::endl; // !

      clinicaActual.nombre = valor;
      leyendoCitas = false;
    }
    else if (tipo == "Doctor") {
      if (!doctorActual.nombre.empty()) {
        clinicaActual.doctores.push_back(doctorActual);
        doctorActual = Doctor();
      }
      std::getline(ss, valor);
      std::cout << "valor // " << valor << std::endl; // !
      doctorActual.nombre = valor;
      leyendoCitas = false;
    }
    else if (tipo == "ID") {
      std::getline(ss, valor);
      std::cout << "valor // " << valor << std::endl; // !
      doctorActual.idDoctor = std::stoi(valor);
    }
    else if (tipo == "Horario") {
      std::string horaEntradaStr, horaSalidaStr;
      int horaEntradaInt, horaSalidaInt;
      std::getline(ss, horaEntradaStr, '-');
      std::getline(ss, horaSalidaStr);

      std::cout << "horaEntrada // " << horaEntradaStr << std::endl; // !
      std::cout << "horaSalida // " << horaSalidaStr << std::endl; // !
      
      horaEntradaInt = std::stoi(horaEntradaStr);
      horaSalidaInt = std::stoi(horaSalidaStr);

      doctorActual.horaEntrada = horaEntradaInt;
      doctorActual.horaSalida = horaSalidaInt;
    }
    else if (tipo == "Dias de Trabajo") {
      doctorActual.diasTrabajo.clear();
      std::string dia;
      int diaInt;
      while ( std::getline(ss, dia, ' ') ) {
        diaInt = std::stoi(dia);
        doctorActual.diasTrabajo.push_back(diaInt);
      }
    }
    else if (tipo == "Citas") {
      leyendoCitas = true;
    }
    else if (leyendoCitas && tipo == "Cita") {
      std::string fechaHoraStr, nombrePacienteStr, etiqueta;
      std::getline(ss, fechaHoraStr, ',');
      citaActual.fechaHora = parseFechaHora(fechaHoraStr);
      
      std::cout << "fechaHoraStr // " << fechaHoraStr << std::endl; // !
      
      std::getline(ss, etiqueta, ':');
      std::getline(ss, nombrePacienteStr);
      std::cout << "nombrePacienteStr // " << nombrePacienteStr << std::endl; // !
      citaActual.nombrePaciente = nombrePacienteStr;
      doctorActual.citas.push_back(citaActual);
    }
  }

  if (!doctorActual.nombre.empty()) {
    clinicaActual.doctores.push_back(doctorActual);
  }
  if (!clinicaActual.nombre.empty()) {
    clinicas.push_back(clinicaActual);
  }

  archivo.close();
}


// ===============================================

// ~~~ Listadores/mostradores/funciones listado ~~~

void listar_clinicas() {

  separador(2);
  if (clinicas.empty()) {
    std::cout << "Aun no hay clinicas registradas" << std::endl;
    return;
  }

  int contador = 1;
  for (auto clinica : clinicas) {
    std::cout << "~~~~~  " << contador << "  ~~~~~" << std::endl;
    std::cout << "Nombre de la clinica: " << clinica.nombre << std::endl;
    std::cout << "Tiene " << clinica.doctores.size() << " doctores asignados" << std::endl;
    contador++;
  }
}

void listar_doctores() {

  separador(2);
  bool hayDoctores = false;
  if (!clinicas.empty()) {
    for (auto clinica : clinicas) {
      if (!clinica.doctores.empty()) {
        hayDoctores = true;
        break;
      }
    }
  }

  if (!hayDoctores) {
    std::cout << "No hay doctores registrados aun." << std::endl;
    return;
  }

  int contador = 1;
  for (auto clinica : clinicas) {
    std::cout << "~~~  " << contador << "  ~~~" << std::endl;

    if ( clinica.doctores.empty()){
      std::cout << "La clinica: " << clinica.nombre << ", No tiene doctores registrados:" << std::endl;
      continue;  
    }

    std::cout << "La clinica: " << clinica.nombre << ", tiene a los doctores:" << std::endl;

    int contadorDoctores = 1;
    for (auto doctor : clinica.doctores) {
      std::cout << contadorDoctores << ". " << doctor.nombre << " (atiende de " << doctor.horaEntrada << " a " << doctor.horaSalida << ")" << std::endl;
      contadorDoctores++;
    }
    contador++;
  }
}

void listar_citas() {

  separador(2);
  bool hayCitas = false;
  if (!clinicas.empty()) {
    for (auto clinica : clinicas) {
      if (!clinica.doctores.empty()) {
        for (auto doctor : clinica.doctores) {
          if (!doctor.citas.empty()) {
            hayCitas = true;
            break;
          }
        }
      }
    }
  }

  if (!hayCitas) {
    std::cout << "No hay citas registradas aun." << std::endl;
    return;
  }


  int contador = 1;
  for (auto clinica : clinicas) {
    std::cout << "~~~  " << contador << "  ~~~" << std::endl;
    std::cout << "Nombre de la clinica: " << clinica.nombre << std::endl;
    int contadorDoctores = 1;
    if (clinica.doctores.empty()){
      std::cout << "No tiene doctores registrados" << std::endl;
      continue;
    }
    for (auto doctor : clinica.doctores) {
      int contadorCitas = 1;
      std::cout << contadorDoctores << ". " << doctor.nombre << std::endl;
      if (doctor.citas.empty()){
        std::cout << "Este doctor no tiene citas registradas" << std::endl;
        continue;
      }
      for (auto cita : doctor.citas) {
        std::cout << "  " << contadorDoctores << "." << contadorCitas << ". Cita de: " << cita.nombrePaciente << std::endl;
        contadorCitas++;
      }
      contadorDoctores++;
    }
  }
}

// ~ ===== Creadores/generadores base =====
Doctor crearDoctor() {

  Doctor nuevoDoctor;
  std::string nombreDoctor;
  int diaInicio, diaFin;
  int horaEntrada, horaSalida;

  do {
    separador(1);
    std::cout << "Ingrese el nombre del doctor: ";
    std::getline(std::cin >> std::ws, nombreDoctor);

    if (busca_nombre_doctor(nombreDoctor)) {
      std::cout << "Ya hay un doctor con el nombre " << nombreDoctor << ". Por favor escribe otro nombre" << std::endl;
      continue;
    }

    std::cout << "Ingrese el dia de inicio de la semana laboral (1 = Lunes, 7 = Domingo): ";
    diaInicio = pedir_numero();
    std::cout << "Ingrese el dia de fin de la semana laboral (1 = Lunes, 7 = Domingo): ";
    diaFin = pedir_numero();

    if (!validaDiasTrabajo(diaInicio, diaFin)) {
      std::cout << "Dias seleccionados invalidos.\n";
      return nuevoDoctor;
    }

    std::cout << "Ingrese la hora de entrada (0-23): ";
    horaEntrada = pedir_numero();
    if (!esHoraValida(horaEntrada)) {
      std::cout << "Hora de entrada invalida.\n";
      return nuevoDoctor;
    }

    std::cout << "Ingrese la hora de salida (0-23): ";
    horaSalida = pedir_numero();
    if (!esHoraValida(horaSalida) || horaSalida <= horaEntrada) {
      std::cout << "Hora de salida invalida.\n";
      return nuevoDoctor;
    }


    // agrega diaInicio... diaFIn
    for (int dia = diaInicio; dia <= diaFin; ++dia) {
      nuevoDoctor.diasTrabajo.push_back(dia);
    }
    nuevoDoctor.nombre = nombreDoctor;
    nuevoDoctor.horaEntrada = horaEntrada;
    nuevoDoctor.horaSalida = horaSalida;
    nuevoDoctor.idDoctor = creaIdDoctor();

    nombresDoctores.push_back(nombreDoctor);
    return nuevoDoctor;

  } while (true);

}

Clinica crearClinica() {

  do {
    separador(1);
    Clinica nuevaClinica;
    std::string nombeClinica;
    std::cout << "Ingrese el nombre de la clinica:(-1 salir) ";
    std::getline(std::cin >> std::ws, nombeClinica);

    if (nombeClinica == "-1") {
      return nuevaClinica;
    }

    nuevaClinica.nombre = nombeClinica;

    return nuevaClinica;
  } while (true);

}

Cita crearCita() {

  Cita nuevaCita;
  std::string nombrePaciente;
  bool primeraVez = true;

  if (clinicas.empty()) {
    std::cout << "No hay clinicas disponibles" << std::endl;
    return nuevaCita;
  }

  do {
    separador(1);
    std::cout << "Seleccione una clinica:\n";
    for (size_t i = 0; i < clinicas.size(); ++i) {
      std::cout << i + 1 << ". " << clinicas[i].nombre << "\n";
    }

    size_t indiceClinica;
    std::cout << "Ingrese el numero de la clinica: ";
    indiceClinica = pedir_numero();

    if (indiceClinica < 1 || indiceClinica > clinicas.size()) {
      std::cout << "Seleccion invalida.\n";
      return nuevaCita;
    }

    indiceClinica--; // Ajustar a indice base 0
    Clinica& clinicaSeleccionada = clinicas[indiceClinica];

    if (clinicaSeleccionada.doctores.empty()) {
      std::cout << "No hay doctores disponibles en esta clinica.\n";
      return nuevaCita;
    }

    std::cout << "Seleccione un doctor:\n";
    for (size_t i = 0; i < clinicaSeleccionada.doctores.size(); ++i) {
      std::cout << i + 1 << ". " << clinicaSeleccionada.doctores[i].nombre << "\n";
    }

    size_t indiceDoctor;
    std::cout << "Ingrese el numero del doctor: ";
    indiceDoctor = pedir_numero();

    if (indiceDoctor < 1 || indiceDoctor > clinicaSeleccionada.doctores.size()) {
      std::cout << "Seleccion invalida.\n";
      return nuevaCita;
    }

    indiceDoctor--; // Ajustar a indice base 0
    Doctor& doctorSeleccionado = clinicas[indiceClinica].doctores[indiceDoctor];

    std::chrono::system_clock::time_point fechaHoraCita;
    bool fechaValida = false;
    while (!fechaValida) {

      int hora;
      separador(1);
      if (primeraVez) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        primeraVez = false;
      }
      std::cout << "Ingresa la fecha y hora de la cita YYYY-MM-DD HH: (-1 para salir) ";
      std::string fechaHoraStr;
      std::getline(std::cin, fechaHoraStr);

      if (fechaHoraStr == "-1") {
        std::cout << "saliendo..." << std::endl;
        return nuevaCita;
      }

      std::cout << "Ingrese el nombre del paciente: ";
      std::getline(std::cin >> std::ws, nombrePaciente);

      try {
        std::tm tm = {};
        std::istringstream iss(fechaHoraStr);
        iss >> std::get_time(&tm, "%Y-%m-%d %H");
        if (iss.fail()) {
          throw std::runtime_error("Formato de fecha invalido.");
        }

        fechaHoraCita = std::chrono::system_clock::from_time_t(std::mktime(&tm));

      }
      catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
      }

      if (esHorarioValidoParaDoctor(doctorSeleccionado, fechaHoraCita) && estaDisponibleElDoctor(doctorSeleccionado, fechaHoraCita)) {
        fechaValida = true;
      }
      else {

        if (!esHorarioValidoParaDoctor(doctorSeleccionado, fechaHoraCita)) {
          std::cout << "No es un horario valido par el doctor" << std::endl;
          continue;
        }

        if (!estaDisponibleElDoctor(doctorSeleccionado, fechaHoraCita)) {
          std::cout << "El doctor ya tiene esa hora ocupada" << std::endl;
          continue;
        }
      }
    }


    nuevaCita.fechaHora = fechaHoraCita;
    nuevaCita.nombrePaciente = nombrePaciente;

    clinicas[indiceClinica].doctores[indiceDoctor].citas.push_back(nuevaCita);
    return nuevaCita;

  } while (true);

}

Cita crearCitaEditada() {

  Cita citaVacia;
  bool primeraVez = false;

  if (clinicas.empty()) {
    std::cout << "No hay clinicas disponibles.\n";
    return citaVacia;
  }

  // Paso 1: Listar clinicas que tienen doctores con citas
  std::cout << "Clinicas con citas agendadas:\n";
  for (size_t i = 0; i < clinicas.size(); ++i) {
    bool tieneCitas = false;
    for (const auto& doctor : clinicas[i].doctores) {
      if (!doctor.citas.empty()) {
        tieneCitas = true;
        break;
      }
    }
    if (tieneCitas) {
      std::cout << i + 1 << ". " << clinicas[i].nombre << "\n";
    }
  }

  // Paso 2: Seleccionar clinica
  size_t indiceClinica;
  std::cout << "Seleccione el indice de la clinica: ";
  indiceClinica = pedir_numero();
  if (indiceClinica < 1 || indiceClinica > clinicas.size()) {
    std::cout << "Seleccion invalida.\n";
    return citaVacia;
  }
  indiceClinica--; // Ajuste a indice base 0

  // Paso 3: Listar doctores de la clinica seleccionada con citas
  std::cout << "Doctores con citas en " << clinicas[indiceClinica].nombre << ":\n";
  for (size_t j = 0; j < clinicas[indiceClinica].doctores.size(); ++j) {
    if (!clinicas[indiceClinica].doctores[j].citas.empty()) {
      std::cout << j + 1 << ". " << clinicas[indiceClinica].doctores[j].nombre << "\n";
    }
  }

  // Paso 4: Seleccionar doctor
  size_t indiceDoctor;
  std::cout << "Seleccione el indice del doctor: ";
  indiceDoctor = pedir_numero();
  if (indiceDoctor < 1 || indiceDoctor > clinicas[indiceClinica].doctores.size()) {
    std::cout << "Seleccion invalida.\n";
    return citaVacia;
  }
  indiceDoctor--; // Ajuste a indice base 0
  Doctor& doctorSeleccionado = clinicas[indiceClinica].doctores[indiceDoctor];

  // Paso 5: Mostrar citas del doctor seleccionado
  std::cout << "Citas de " << clinicas[indiceClinica].doctores[indiceDoctor].nombre << ":\n";
  for (size_t k = 0; k < clinicas[indiceClinica].doctores[indiceDoctor].citas.size(); ++k) {
    std::cout << k + 1 << ". Fecha: " << formatearFechaHora(clinicas[indiceClinica].doctores[indiceDoctor].citas[k].fechaHora) << ". Paciente de:" << clinicas[indiceClinica].doctores[indiceDoctor].citas[k].nombrePaciente << "\n";
  }

  size_t indiceCita;
  std::cout << "Seleccione el numero de cita: ";
  indiceCita = pedir_numero();
  if (indiceCita < 1 || indiceCita > clinicas[indiceClinica].doctores[indiceDoctor].citas.size()) {
    std::cout << "Seleccion invalida.\n";
    return citaVacia;
  }
  indiceCita--; // Ajuste a indice base 0
  Cita citaEditando = clinicas[indiceClinica].doctores[indiceDoctor].citas[indiceCita];


  std::string nombrePaciente;
  std::chrono::system_clock::time_point fechaHoraCita;
  bool fechaValida = false;
  while (!fechaValida) {

    int hora;
    separador(1);
    if (primeraVez) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      primeraVez = false;
    }
    std::cout << "Ingresa la fecha y hora de la cita YYYY-MM-DD HH: (-1 para salir) ";
    std::string fechaHoraStr;
    std::getline(std::cin, fechaHoraStr);

    if (fechaHoraStr == "-1") {
      std::cout << "saliendo..." << std::endl;
      return citaVacia;
    }


    std::cout << "Ingrese el nombre del paciente: ";
    std::getline(std::cin >> std::ws, nombrePaciente);

    try {
      std::tm tm = {};
      std::istringstream iss(fechaHoraStr);
      iss >> std::get_time(&tm, "%Y-%m-%d %H");
      if (iss.fail()) {
        throw std::runtime_error("Formato de fecha invalido.");
      }

      fechaHoraCita = std::chrono::system_clock::from_time_t(std::mktime(&tm));


    }
    catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }

    if (esHorarioValidoParaDoctor(doctorSeleccionado, fechaHoraCita) && estaDisponibleElDoctorEditar(doctorSeleccionado, fechaHoraCita, indiceCita)) {
      fechaValida = true;
    }
    else {

      if (!esHorarioValidoParaDoctor(doctorSeleccionado, fechaHoraCita)) {
        std::cout << "No es un horario valido par el doctor" << std::endl;
        continue;
      }

      if (!estaDisponibleElDoctorEditar(doctorSeleccionado, fechaHoraCita, indiceCita)) {
        std::cout << "El doctor ya tiene esa hora ocupada" << std::endl;
        continue;
      }
    }
  }


  citaEditando.fechaHora = fechaHoraCita;
  citaEditando.nombrePaciente = nombrePaciente;

  return citaEditando;

}

// Prototipos de funciones
void menuClinicas();
void agregarClinica();
void eliminarClinica();
void editarClinica();
void menuCitas();
void agregarCita();
void eliminarCita();
void editarCita();

int main() {

  cargarDatos();

  int opcion;
  do {
    separador();
    std::cout << "==== Menu principal ====" << std::endl;
    std::cout << "1. Menu de Clinicas\n";
    std::cout << "2. Menu de Citas\n\n";
    std::cout << "~ Listados ~\n";
    std::cout << "3. Listar clinicas\n";
    std::cout << "4. Listar doctores\n";
    std::cout << "5. Listar citas\n";
    std::cout << "~~ ~~ ~~ ~~ ~~\n";
    std::cout << "6. Salir\n";
    std::cout << "Seleccione una opcion: ";
    opcion = pedir_numero();

    switch (opcion) {
    case 1:
      menuClinicas();
      break;
    case 2:
      menuCitas();
      break;
    case 3:
      listar_clinicas();
      break;
    case 4:
      listar_doctores();
      break;
    case 5:
      listar_citas();
      break;
    case 6:
      return 0;
      break;
    default:
      std::cout << "Opcion no valida. Intente de nuevo.\n";
    }
  } while (opcion != 6);

  return 0;
}

void menuClinicas() {
  int opcion;
  do {
    separador();
    std::cout << "==== Menu de Clinicas ====\n";
    std::cout << "1. Agregar Clinica\n";
    std::cout << "2. Eliminar Clinica\n";
    std::cout << "3. Editar Clinica\n";
    std::cout << "4. Salir\n";
    std::cout << "Seleccione una opcion: ";
    opcion = pedir_numero();

    switch (opcion) {
    case 1:
      agregarClinica();
      break;
    case 2:
      eliminarClinica();
      break;
    case 3:
      editarClinica();
      break;
    case 4:
      break;
    default:
      std::cout << "Opcion no valida. Intente de nuevo.\n";
    }
  } while (opcion != 4);
}

void agregarClinica() {
  Clinica nuevaClinica;

  nuevaClinica = crearClinica();

  if (nuevaClinica.nombre != "") { // valida que la clinica tenga nombre, significa que le ha devuelto una clinica con nombre
    clinicas.push_back(nuevaClinica);
    std::cout << "Clinica agregada con exito." << std::endl;
    guardarDatos();
  }


}

void eliminarClinica() {

  do {

    if (clinicas.empty()) {
      std::cout << "No hay clinicas disponibles para eliminar.\n";
      return;
    }

    separador(1);
    std::cout << "Seleccione una clinica para eliminar:\n";
    for (size_t i = 0; i < clinicas.size(); ++i) {
      std::cout << i + 1 << ". " << clinicas[i].nombre << "\n";
    }

    size_t indiceClinica;
    std::cout << "Ingrese el numero de la clinica: ";
    indiceClinica = pedir_numero();

    if (indiceClinica < 1 || indiceClinica > clinicas.size()) {
      std::cout << "Seleccion invalida, seleccione una de las clinicas listadas por favor\n";
      continue;
    }

    indiceClinica--;
    clinicas.erase(clinicas.begin() + indiceClinica);
    guardarDatos();
    return;

  } while (true);

}

void editarClinica() {
  if (clinicas.empty()) {
    std::cout << "No hay clinicas disponibles para editar.\n";
    return;
  }

  separador(1);
  std::cout << "Seleccione una clinica para editar (-1 salir):\n";
  for (size_t i = 0; i < clinicas.size(); ++i) {
    std::cout << i + 1 << ". " << clinicas[i].nombre << "\n";
  }

  size_t indiceClinica;
  std::cout << "Ingrese el numero de la clinica: ";
  indiceClinica = pedir_numero();

  if (indiceClinica == -1) {
    std::cout << "saliendo..." << std::endl;
    return;
  }

  if (indiceClinica < 1 || indiceClinica > clinicas.size()) {
    std::cout << "Seleccion invalida.\n";
    return;
  }

  indiceClinica--; // Ajustar a indice base 0

  int opcion;
  separador(1);
  std::cout << "==== Menu edicion clinicas ====" << std::endl;
  std::cout << "1. Editar nombre de la clinica\n";
  std::cout << "2. Agregar doctores a la clinica\n";
  std::cout << "3. Eliminar doctores a la clinica\n";
  std::cout << "4. Editar doctores de la clinica\n";
  std::cout << "5. Salir\n";
  std::cout << "Seleccione una opcion: ";
  opcion = pedir_numero();

  switch (opcion)
  {
  case 1: {

    do {
      std::string nuevoNombreClinica;
      std::cout << "Ingrese el nuevo nombre de la clinica:(-1 cancelar) ";
      std::getline(std::cin >> std::ws, nuevoNombreClinica);

      if ( nuevoNombreClinica == "-1" ){
        return;
      }
      
      std::cout << "nuevoNombreClinica " << nuevoNombreClinica << std::endl;

      clinicas[indiceClinica].nombre = nuevoNombreClinica;
      std::cout << "Nombre de clinica actualizado.\n";

      guardarDatos();
      break;
    } while (true);

    break;}
  case 2: {

    Doctor nuevoDoctor;

    nuevoDoctor = crearDoctor();

    if (nuevoDoctor.nombre == "") {
      return;
    }

    clinicas[indiceClinica].doctores.push_back(nuevoDoctor);
    std::cout << "Doctor agregado exitosamente." << std::endl;
    guardarDatos();

    break;}
  case 3: {

    if (clinicas[indiceClinica].doctores.empty()) {
      std::cout << "No hay doctores asignados a esta clinica." << std::endl;
      return;
    }

    separador(1);
    std::cout << "Doctores en la clinica:\n";
    int contador = 1;
    for (const auto& doctor : clinicas[indiceClinica].doctores) {
      std::cout << contador << ".- " << doctor.nombre << "\n";
      contador++;
    }

    std::string nombreDoctor;
    std::cout << "Ingrese el nombre del doctor a eliminar: ";
    std::getline(std::cin >> std::ws, nombreDoctor);

    auto& doctoresClinica = clinicas[indiceClinica].doctores;
    auto it = std::find_if(doctoresClinica.begin(), doctoresClinica.end(), [&](const Doctor& d) { return d.nombre == nombreDoctor; });

    if (it != doctoresClinica.end()) {
      Doctor& doctorEncontrado = *it; // Obtener el doctor encontrado

      if (!doctorEncontrado.citas.empty()) {
        std::cout << "El doctor tiene citas, no puede eliminarse sin haber eliminado sus citas." << std::endl;
        return;
      }

      doctoresClinica.erase(it);
      std::cout << "Doctor eliminado de la clinica.\n";
      guardarDatos();
    }
    else {
      std::cout << "No se encontro un doctor con ese nombre en la clinica.\n";
    }


    break;}
  case 4: {
    if (clinicas[indiceClinica].doctores.empty()) {
      std::cout << "No hay doctores asignados a esta clinica." << std::endl;
      return;
    }

    auto doctores = clinicas[indiceClinica].doctores;

    separador(1);
    std::cout << "Seleccione un doctor para editar:\n";
    for (size_t i = 0; i < doctores.size(); ++i) {
      std::cout << i + 1 << ". " << doctores[i].nombre << "\n";
    }

    size_t indiceDoctor;
    std::cout << "Ingrese el numero del doctor: ";
    indiceDoctor = pedir_numero();

    if (indiceDoctor < 1 || indiceDoctor > doctores.size()) {
      std::cout << "Seleccion invalida.\n";
      return;
    }

    // Ajustar el indice para el acceso al vector (base 0)
    indiceDoctor--;

    Doctor& doctorSeleccionado = clinicas[indiceClinica].doctores[indiceDoctor];

    if (!doctorSeleccionado.citas.empty()) {
      std::cout << "No se puede editar un doctor con citas, libera sus citas antes de editarlo.\n" << std::endl;
      return;
    }

    std::cout << "== Proceso de edicion de doctor ==" << std::endl;
    Doctor nuevoDoctor = crearDoctor();

    if (nuevoDoctor.nombre == "") {
      return;
    }
    eliminarNombreDoctor(clinicas[indiceClinica].doctores[indiceDoctor].nombre);
    clinicas[indiceClinica].doctores[indiceDoctor] = nuevoDoctor;
    guardarDatos();

    break;}
  case 5: {
    return;
    break;}

  default:
    break;
  }

}

void menuCitas() {
  int opcion;
  do {
    separador();
    std::cout << "==== Menu de Citas ====\n";
    std::cout << "1. Agregar Cita\n";
    std::cout << "2. Eliminar Cita\n";
    std::cout << "3. Editar Cita\n";
    std::cout << "4. Salir\n";
    std::cout << "Seleccione una opcion: ";
    opcion = pedir_numero();

    switch (opcion) {
    case 1:
      agregarCita();
      break;
    case 2:
      eliminarCita();
      break;
    case 3:
      editarCita();
      break;
    case 4:
      break;
    default:
      std::cout << "Opcion no valida. Intente de nuevo.\n";
    }
  } while (opcion != 4);
}

void agregarCita() {
  Cita nuevaCita;

  nuevaCita = crearCita();

  if (nuevaCita.nombrePaciente == "") {
    return;
  }

  std::cout << "Cita creada con exito." << std::endl;
  guardarDatos();

}

void eliminarCita() {
  if (clinicas.empty()) {
    std::cout << "No hay clinicas disponibles.\n";
    return;
  }

  // Paso 1: Listar clinicas que tienen doctores con citas
  std::cout << "Clinicas con citas agendadas:\n";
  for (size_t i = 0; i < clinicas.size(); ++i) {
    bool tieneCitas = false;
    for (const auto& doctor : clinicas[i].doctores) {
      if (!doctor.citas.empty()) {
        tieneCitas = true;
        break;
      }
    }
    if (tieneCitas) {
      std::cout << i + 1 << ". " << clinicas[i].nombre << "\n";
    }
  }

  // Paso 2: Seleccionar clinica
  size_t indiceClinica;
  std::cout << "Seleccione el indice de la clinica: ";
  indiceClinica = pedir_numero();
  if (indiceClinica < 1 || indiceClinica > clinicas.size()) {
    std::cout << "Seleccion invalida.\n";
    return;
  }
  indiceClinica--; // Ajuste a indice base 0

  // Paso 3: Listar doctores de la clinica seleccionada con citas
  std::cout << "Doctores con citas en " << clinicas[indiceClinica].nombre << ":\n";
  for (size_t j = 0; j < clinicas[indiceClinica].doctores.size(); ++j) {
    if (!clinicas[indiceClinica].doctores[j].citas.empty()) {
      std::cout << j + 1 << ". " << clinicas[indiceClinica].doctores[j].nombre << "\n";
    }
  }

  // Paso 4: Seleccionar doctor
  size_t indiceDoctor;
  std::cout << "Seleccione el indice del doctor: ";
  indiceDoctor = pedir_numero();
  if (indiceDoctor < 1 || indiceDoctor > clinicas[indiceClinica].doctores.size()) {
    std::cout << "Seleccion invalida.\n";
    return;
  }
  indiceDoctor--; // Ajuste a indice base 0

  // Paso 5: Mostrar citas del doctor seleccionado
  std::cout << "Citas de " << clinicas[indiceClinica].doctores[indiceDoctor].nombre << ":\n";
  for (size_t k = 0; k < clinicas[indiceClinica].doctores[indiceDoctor].citas.size(); ++k) {
    std::cout << k + 1 << ". Fecha: " << formatearFechaHora(clinicas[indiceClinica].doctores[indiceDoctor].citas[k].fechaHora) << "\n";
  }

  size_t indiceCita;
  std::cout << "Seleccione el numero de cita: ";
  indiceCita = pedir_numero();
  if (indiceCita < 1 || indiceCita > clinicas[indiceClinica].doctores[indiceDoctor].citas.size()) {
    std::cout << "Seleccion invalida.\n";
    return;
  }
  indiceCita--; // Ajuste a indice base 0

  clinicas[indiceClinica].doctores[indiceDoctor].citas.erase(clinicas[indiceClinica].doctores[indiceDoctor].citas.begin() + indiceCita);
  std::cout << "Eliminacion de cita concretada." << std::endl;
  guardarDatos();

}

void editarCita() {

  Cita nuevaCita = crearCitaEditada();

  if (nuevaCita.nombrePaciente == "") {
    return;
  }

  std::cout << "Edicion de cita exitosa" << std::endl;
  guardarDatos();

}