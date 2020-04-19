#ifndef TESTVR_GLOBAL_H
#define TESTVR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TESTVR_LIBRARY)
#  define TESTVRSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TESTVRSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TESTVR_GLOBAL_H
