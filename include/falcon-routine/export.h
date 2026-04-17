#ifndef FALCON_ROUTINE_EXPORT_H
#define FALCON_ROUTINE_EXPORT_H

#ifdef _WIN32
// Building the DLL
#ifdef falcon_core_EXPORTS
#define FALCON_ROUTINE_API __declspec(dllexport)
#else
// Using the DLL
#define FALCON_ROUTINE_API __declspec(dllimport)
#endif
#else
// Unix/Linux - use visibility attributes for better control
#if defined(__GNUC__) || defined(__clang__)
#define FALCON_ROUTINE_API __attribute__((visibility("default")))
#else
#define FALCON_ROUTINE_API
#endif
#endif

#endif // FALCON_ROUTINE_EXPORT_H
