#include <stdio.h>
#include <inttypes.h>

int main(int argc, char *argv[])
{
	uint8_t out[256][7] = {{0}};
	char buf[256];
	FILE *fp;
	int i;

	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "could not open %s\n", argv[1]);
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		uint8_t idx = buf[3];
		if (buf[0] != '#' && buf[1] != '#')
			continue;
		fgets(buf, sizeof(buf), fp); /* skip line */
		for (i = 0; i < 7; i++) {
			uint8_t val = 0;
			fgets(buf, sizeof(buf), fp);
			if (buf[1] != '_') val |= 0x01;
			if (buf[3] != '_') val |= 0x02;
			if (buf[5] != '_') val |= 0x04;
			if (buf[7] != '_') val |= 0x08;
			out[idx][i] = val;
		}
	}

	printf("PROGMEM uint8_t fonte[0x100][7] = {\n");
	for (i = 0; i < 0x100; i++)
		printf("%s{ 0x%02x, 0x%02x, 0x%02x, 0x%02x },%c", (i&1) ? "" : "    ",
		       out[i][0]<<4 | out[i][1], out[i][2]<<4 | out[i][3],
		       out[i][4]<<4 | out[i][5], out[i][6]<<4, (i&1) ? '\n' : ' ');
	printf("};\n");

	fclose(fp);

	return 0;
}
