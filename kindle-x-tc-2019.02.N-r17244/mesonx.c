#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stdalign.h>

int main() {
	printf("sizeof int: %zu\n", sizeof(int));
	printf("sizeof wchar_t: %zu\n", sizeof(wchar_t));
	printf("sizeof ptr: %zu\n", sizeof(void*));

	printf("alignof char: %zu\n", alignof(char));
	printf("alignof ptr: %zu\n", alignof(void*));
	printf("alignof double: %zu\n", alignof(double));

	return 0;
}
