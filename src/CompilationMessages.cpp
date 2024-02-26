/**
 * In dieser Datei werden aufgrund von Präprozessorvariablen
 * Informationen über die aktuelle Kompilierung ausgegeben.
 */

#ifndef USE_HCE
#ifndef __AVX2__
#ifndef __SSE4_1__
#pragma message "Using scalar code for NNUE inference. This will be slow. Check if your machine supports the SSE4.1 or AVX2 instruction set."
#endif
#endif
#else
#pragma message "Using HCE (Handcrafted Evaluation) instead of NNUE (Efficiently Updatable Neural Networks) will weaken the engine's playing strength."
#endif

#ifdef DISABLE_THREADS
#pragma message "Multithreading is disabled. The program will not respond to UCI commands while searching."
#endif