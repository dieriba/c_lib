#include "d_types.h"


typedef struct _DStringView DStringView;

/**
 * @brief Represents a view of a string only read operation are allowed.
 *
 * The _DStringView holds a reference to an already allocated string, 
 *
 * @struct _DStringView
 * @param string Pointer to the character array containing the string data.
 * @param len Length of the string.
 */
struct _DStringView {
    char    *string;
	usize     	len;
};

#define String(str) DStringView {.string =  str, .len = sizeof(str)}