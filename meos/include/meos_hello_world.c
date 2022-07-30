/* meos_hello_world.c 
 * To build the program run the following command
 * cc meos_hello_world.c -o meos_hello_world -l meos
 */

#include <stdio.h>  /* for printf */

/* Only the MEOS API header is required */
#include "meos.h"

int main()
{
    /* Initialize session_timezone */
    meos_timezone_initialize("Europe/Brussels");

    /* Read WKT into geometry object */
    Temporal *inst = tgeompoint_in("POINT(1 1)@2000-01-01");
    Temporal *is = tgeompoint_in("{POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02}");
    Temporal *seq = tgeompoint_in("[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]");
    Temporal *ss = tgeompoint_in("{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02],"
      "[POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}");

    /* Convert result to MF-JSON */
    char *inst_wkt = temporal_as_mfjson(inst, true, 6, NULL);
    printf("Temporal instant:\n%s\n", inst_wkt);
    char *is_wkt = temporal_as_mfjson(inst, true, 6, NULL);
    printf("Temporal instant set:\n%s\n", is_wkt);
    char *seq_wkt = temporal_as_mfjson(inst, true, 6, NULL);
    printf("Temporal sequence:\n%s\n", seq_wkt);
    char *ss_wkt = temporal_as_mfjson(inst, true, 6, NULL);
    printf("Temporal sequence set:\n%s\n", ss_wkt);

    /* Clean up allocated objects */
    free(inst); free(inst_wkt);
    free(is); free(is_wkt);
    free(seq); free(seq_wkt);
    free(ss); free(ss_wkt);

    /* Return */
    return 0;
}