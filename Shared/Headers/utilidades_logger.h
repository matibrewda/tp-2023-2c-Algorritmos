#ifndef UTILIDADES_LOGGER_H
#define UTILIDADES_LOGGER_H

#include<commons/log.h>

t_log* crear_logger(char* ruta_archivo_de_logs, char* nombre_modulo, t_log_level log_level);
void destruir_logger(t_log* logger);

#endif /* UTILIDADES_LOGGER_H */