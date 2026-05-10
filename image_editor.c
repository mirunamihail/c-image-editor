//314CA, Mihail Miruna-Gabriela
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
	char type[3];
	int width, height;
	int max_value;
	unsigned char **pixels; //Grayscale
	unsigned char ***colors; //Color
	int is_color;
	int selected_x1, selected_y1, selected_x2, selected_y2; // Selectie
	int is_loaded;
} Image;

int clamp(int value, int min, int max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

double min(double a, double b)
{
	return (a < b) ? a : b;
}

double max(double a, double b)
{
	return (a > b) ? a : b;
}

void free_image(Image *img)
{
	if (img->is_color) {
		if (img->colors) {
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					free(img->colors[i][j]); // Eliberam RGB
				}
				free(img->colors[i]); // Eliberam randul
			}
			free(img->colors); // Eliberam imaginea
		}
	} else {
		if (img->pixels) {
			for (int i = 0; i < img->height; i++) {
				free(img->pixels[i]); // Eliberam randul de pixeli
			}
			free(img->pixels); // Eliberam imaginea
		}
	}
	memset(img, 0, sizeof(Image)); // Face valorile 0
}

void load_image(Image *img, const char *filename)
{
	// Eliberarea imaginii anterioare inainte de citrea fisierului
	free_image(img);
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to load %s\n", filename);
		return;
	}
	fscanf(file, "%s", img->type);
	if (strcmp(img->type, "P2") == 0 || strcmp(img->type, "P5") == 0) {
		img->is_color = 0; // Grayscale
	} else if (strcmp(img->type, "P3") == 0 || strcmp(img->type, "P6") == 0) {
		img->is_color = 1; // Color
	} else {
		fclose(file);
		printf("Failed to load %s\n", filename);
		return;
	}
	// Citire valori
	fscanf(file, "%d %d", &img->width, &img->height);
	fscanf(file, "%d", &img->max_value);
	fgetc(file); // Citeste newline

	// Alocarea memoriei pentru imagine
	if (img->is_color) { // Verificare color/grayscale
		img->colors = malloc(img->height * sizeof(unsigned char **));
		for (int i = 0; i < img->height; i++) {
			img->colors[i] = malloc(img->width * sizeof(unsigned char *));
			for (int j = 0; j < img->width; j++) {
				img->colors[i][j] = malloc(3 * sizeof(unsigned char)); // RGB
			}
		}

		// Citire pixeli
		if (strcmp(img->type, "P3") == 0) { // P3 ascii
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					fscanf(file, "%hhu %hhu %hhu",
						   &img->colors[i][j][0],
						   &img->colors[i][j][1],
						   &img->colors[i][j][2]);
				}
			}
		} else if (strcmp(img->type, "P6") == 0) { // P6 binar
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					fread(&img->colors[i][j][0], 1, 1, file);
					fread(&img->colors[i][j][1], 1, 1, file);
					fread(&img->colors[i][j][2], 1, 1, file);
				}
			}
		}
	} else { // Imaginea alb-negru
		img->pixels = malloc(img->height * sizeof(unsigned char *));
		for (int i = 0; i < img->height; i++) {
			img->pixels[i] = malloc(img->width * sizeof(unsigned char));
		}

		// Citire pixeli
		if (strcmp(img->type, "P2") == 0) { // P2 ascii
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					fscanf(file, "%hhu", &img->pixels[i][j]);
				}
			}
		} else if (strcmp(img->type, "P5") == 0) { // P5 binar
			for (int i = 0; i < img->height; i++) {
				fread(img->pixels[i], sizeof(unsigned char), img->width, file);
			}
		}
	}

	fclose(file); // Inchidem fisierul dupa folosire
	img->is_loaded = 1; // Imaginea este loaded
	printf("Loaded %s\n", filename);
	// Setam selectia pe toata imaginea
	img->selected_x1 = 0;
	img->selected_y1 = 0;
	img->selected_x2 = img->width;
	img->selected_y2 = img->height;
}

void select_region(Image *img, int x1, int y1, int x2, int y2)
{
	if (!img->is_loaded) { // Verif is_loaded la fiecare functie
		printf("No image loaded\n");
		return;
	}
	if (x1 > x2) { // Verifica ordinea si interschimba
		int temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) {
		int temp = y1;
		y1 = y2;
		y2 = temp;
	}
	if (x1 < 0 || x2 > img->width || y1 < 0 ||
		y2 > img->height || x1 == x2 || y1 == y2) {
		printf("Invalid set of coordinates\n");
		return;
	}
	img->selected_x1 = x1; // Atribuie valori
	img->selected_y1 = y1;
	img->selected_x2 = x2;
	img->selected_y2 = y2;

	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

void select_all(Image *img)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	img->selected_x1 = 0;
	img->selected_y1 = 0;
	img->selected_x2 = img->width;
	img->selected_y2 = img->height;

	printf("Selected ALL\n");
}

void rotate(Image *img, int angle)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	int angle_cpy = angle; // Copie pentru afisare
	if (angle < 0) { // Normalizare unghi
		angle = (360 - ((-angle) % 360)) % 360;
	} else {
		angle = (angle % 360 + 360) % 360;
	}
	if (angle != 0 && angle != 90 && angle != 180 && angle != 270) {
		printf("Unsupported rotation angle\n");
		return;
	}
	if (angle_cpy == 0 || angle_cpy == 360 || angle_cpy == -360) {
		printf("Rotated %d\n", angle_cpy);
		return; // Trece mai departe
	}

	// Determinare coordonate selectie
	int x1 = img->selected_x1, y1 = img->selected_y1;
	int x2 = img->selected_x2, y2 = img->selected_y2;
	int width = x2 - x1;
	int height = y2 - y1;

	// Verif daca selectia este pe toata imaginea
	int full_image = (x1 == 0 && y1 == 0 && x2 == img->width && y2 == img->height);

	if (full_image) {
		// In functie de unghi actualizam height si width
		int new_width = (angle == 90 || angle == 270) ? img->height : img->width;
		int new_height = (angle == 90 || angle == 270) ? img->width : img->height;

		if (img->is_color) {
			// Alocare memorie cu noile dimensiuni
			unsigned char ***new_colors;
			new_colors = malloc(new_height * sizeof(unsigned char **));
			for (int i = 0; i < new_height; i++) {
				new_colors[i] = malloc(new_width * sizeof(unsigned char *));
				for (int j = 0; j < new_width; j++) {
					new_colors[i][j] = malloc(3 * sizeof(unsigned char));
				}
			}
			// Rotirea
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					if (angle == 90) {
						new_colors[j][(new_width - 1) - i][0] = img->colors[i][j][0];
						new_colors[j][(new_width - 1) - i][1] = img->colors[i][j][1];
						new_colors[j][(new_width - 1) - i][2] = img->colors[i][j][2];
					} else if (angle == 180) {
						new_colors[(new_height - 1) - i][(new_width - 1) - j][0] = img->colors[i][j][0];
						new_colors[(new_height - 1) - i][(new_width - 1) - j][1] = img->colors[i][j][1];
						new_colors[(new_height - 1) - i][(new_width - 1) - j][2] = img->colors[i][j][2];
					} else if (angle == 270) {
						new_colors[(new_height - 1) - j][i][0] = img->colors[i][j][0];
						new_colors[(new_height - 1) - j][i][1] = img->colors[i][j][1];
						new_colors[(new_height - 1) - j][i][2] = img->colors[i][j][2];
					}
				}
			}
			// Eliberare memorie
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					free(img->colors[i][j]);
				}
				free(img->colors[i]);
			}
			free(img->colors);

			img->colors = new_colors; // Actualizare pointer
		}
		else {
			// Alocare memorie pentru grayscale
			unsigned char **new_pixels = malloc(new_height * sizeof(unsigned char *));
			for (int i = 0; i < new_height; i++) {
				new_pixels[i] = malloc(new_width * sizeof(unsigned char));
			}
			// Rotirea
			for (int i = 0; i < img->height; i++) {
				for (int j = 0; j < img->width; j++) {
					if (angle == 90) {
						new_pixels[j][(new_width - 1) - i] = img->pixels[i][j];
					}
					else if (angle == 180) {
						new_pixels[(new_height - 1) - i][(new_width - 1) - j] = img->pixels[i][j];
					}
					else if (angle == 270) {
						new_pixels[(new_height - 1) - j][i] = img->pixels[i][j];
					}
				}
			}
			// Eliberam memoria
			for (int i = 0; i < img->height; i++) {
				free(img->pixels[i]);
			}
			free(img->pixels);

			img->pixels = new_pixels; // Actualizam pointerul
		}
		// Actualizare dimensiuni
		img->width = new_width;
		img->height = new_height;
		// Resetare selectie pe toata poza
		img->selected_x1 = 0;
		img->selected_y1 = 0;
		img->selected_x2 = img->width;
		img->selected_y2 = img->height;
		printf("Rotated %d\n", angle_cpy);
		return;
	}
	// Daca nu e toata imaginea verificam selectia patrata
	if (width != height) {
		printf("The selection must be square\n");
		return;
	}
	// Rotirea pe selectie
	if (img->is_color) {
		// Alocare memorie
		unsigned char ***temp_colors = malloc(width * sizeof(unsigned char **));
		for (int i = 0; i < width; i++) {
			temp_colors[i] = malloc(width * sizeof(unsigned char *));
			for (int j = 0; j < width; j++) {
				temp_colors[i][j] = malloc(3 * sizeof(unsigned char));
			}
		}
		// Rotirea
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				if (angle == 90) {
					for (int c = 0; c < 3; c++) {
						temp_colors[j][width - 1 - i][c] = img->colors[y1 + i][x1 + j][c];
					}
				} else if (angle == 180) {
					for (int c = 0; c < 3; c++) {
						temp_colors[width - 1 - i][width - 1 - j][c] = img->colors[y1 + i][x1 + j][c];
					}
				} else if (angle == 270) {
					for (int c = 0; c < 3; c++) {
						temp_colors[width - 1 - j][i][c] = img->colors[y1 + i][x1 + j][c];
					}
				}
			}
		}

		// Copiere in imagine
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				for (int c = 0; c < 3; c++) {
					img->colors[y1 + i][x1 + j][c] = temp_colors[i][j][c];
				}
			}
		}

		// Eliberarea memoriei
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				free(temp_colors[i][j]);
			}
			free(temp_colors[i]);
		}
		free(temp_colors);
	} else {
		// Grayscale si alocare de memorie
		unsigned char **temp_pixels = malloc(width * sizeof(unsigned char *));
		for (int i = 0; i < width; i++) {
			temp_pixels[i] = malloc(width * sizeof(unsigned char));
		}
		// Rotirea
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				if (angle == 90) {
					temp_pixels[j][width - 1 - i] = img->pixels[y1 + i][x1 + j];
				} else if (angle == 180) {
					temp_pixels[width - 1 - i][width - 1 - j] = img->pixels[y1 + i][x1 + j];
				} else if (angle == 270) {
					temp_pixels[width - 1 - j][i] = img->pixels[y1 + i][x1 + j];
				}
			}
		}

		// Copiere in poza
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				img->pixels[y1 + i][x1 + j] = temp_pixels[i][j];
			}
		}

		// Eliberarea memoriei
		for (int i = 0; i < width; i++) {
			free(temp_pixels[i]);
		}
		free(temp_pixels);
	}

	printf("Rotated %d\n", angle_cpy);
}

void crop(Image *img)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	// Selectia
	int width = img->selected_x2 - img->selected_x1;
	int height = img->selected_y2 - img->selected_y1;

	if (img->is_color) {
		// Alocare memorie pt noua imagine color
		unsigned char ***new_colors = malloc(height * sizeof(unsigned char **));
		for (int i = 0; i < height; i++) {
			new_colors[i] = malloc(width * sizeof(unsigned char *));
			for (int j = 0; j < width; j++) {
				new_colors[i][j] = malloc(3 * sizeof(unsigned char));
				for (int k = 0; k < 3; k++) {
					new_colors[i][j][k] = img->colors[img->selected_y1 + i][img->selected_x1 + j][k];
				}
			}
		}

		// Eliberarea vechii imagini
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				free(img->colors[i][j]);
			}
			free(img->colors[i]);
		}
		free(img->colors);

		img->colors = new_colors; // Actualizare imagine color
	} else {
		// Alocare memorie pt noua imagine grayscale
		unsigned char **new_pixels = malloc(height * sizeof(unsigned char *));
		for (int i = 0; i < height; i++) {
			new_pixels[i] = malloc(width * sizeof(unsigned char));
			for (int j = 0; j < width; j++) {
				new_pixels[i][j] = img->pixels[img->selected_y1 + i][img->selected_x1 + j];
			}
		}

		// Eliberarea vechii imagini
		for (int i = 0; i < img->height; i++) {
			free(img->pixels[i]);
		}
		free(img->pixels);

		img->pixels = new_pixels; // Actualizare imagine grayscale
	}

	// Actualizeaza valorile dimensiunilor
	img->width = width;
	img->height = height;

	// Resetarea selectiei
	img->selected_x1 = 0;
	img->selected_y1 = 0;
	img->selected_x2 = width;
	img->selected_y2 = height;

	printf("Image cropped\n");
}

void equalize(Image *img) 
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	// Verificare grayscale
	if (img->is_color) {
		printf("Black and white image needed\n");
		return;
	}

	double area = 1.0 * img->width * img->height;
	double histogram[256] = {0};
	double cumulative_histogram[256] = {0};

	// Calculare histograma
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			histogram[img->pixels[i][j]]++;
		}
	}

	// Calculare histograma cumulativa
	cumulative_histogram[0] = histogram[0];
	for (int i = 1; i < 256; i++) {
		cumulative_histogram[i] = cumulative_histogram[i - 1] + histogram[i];
	}

	// Egalizarea imaginii
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			int value = img->pixels[i][j];
			double new_value = (cumulative_histogram[value] * 255.0) / area;
			new_value = max(0.0, min(255.0, new_value));
			img->pixels[i][j] = (unsigned char)round(new_value);
		}
	}
	printf("Equalize done\n");
}

void histogram(Image *img, int x, int y)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	// Verificare grayscale
	if (img->is_color) {
		printf("Black and white image needed\n");
		return;
	}

	if (y < 2 || y > 256 || (y & (y - 1)) != 0) { // Verifcare daca y este puterea lui 2
		printf("Invalid set of parameters\n");
		return;
	}

	int bin_size = 256 / y;
	int histogram[y];
	memset(histogram, 0, sizeof(histogram)); // Face toate valorile 0

	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			int value = img->pixels[i][j]; // Valoare pixelului
			int bin_index = value / bin_size; // In ce bin se afla pixelul
			histogram[bin_index]++;
		}
	}

	// Calculam binul maxim pentru afisare
	int max_frequency = 0;
	for (int i = 0; i < y; i++) {
		if (histogram[i] > max_frequency) {
			max_frequency = histogram[i];
		}
	}

	// Afisare histograma prin parcurgerea binilor
	for (int i = 0; i < y; i++) {
		int num_stars = (int)((double)histogram[i] / max_frequency * x);
		printf("%d\t|\t", num_stars); // Nr de stele pentru binul i
		for (int j = 0; j < num_stars; j++) {
			printf("*");
		}
		printf("\n");
	}
}

void apply_filter(Image *img, const char *filter_name)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}
	int x1 = img->selected_x1;
	int x2 = img->selected_x2;
	int y1 = img->selected_y1;
	int y2 = img->selected_y2;
	if (!img->is_color) { // Verifica culoare
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	// Definirea nucleelor si le copierea in kernel
	int kernel[3][3] = {0};
	if (strcmp(filter_name, "EDGE") == 0) {
		int edge[3][3] = {
			{-1, -1, -1},
			{-1,  8, -1},
			{-1, -1, -1}
		};
		memcpy(kernel, edge, sizeof(kernel));
	} else if (strcmp(filter_name, "SHARPEN") == 0) {
		int sharpen[3][3] = {
			{ 0, -1,  0},
			{-1,  5, -1},
			{ 0, -1,  0}
		};
		memcpy(kernel, sharpen, sizeof(kernel));
	} else if (strcmp(filter_name, "BLUR") == 0) {
		int blur[3][3] = {
			{1, 1, 1},
			{1, 1, 1},
			{1, 1, 1}
		};
		memcpy(kernel, blur, sizeof(kernel));
	} else if (strcmp(filter_name, "GAUSSIAN_BLUR") == 0) {
		int gaussian_blur[3][3] = {
			{1, 2, 1},
			{2, 4, 2},
			{1, 2, 1}
		};
		memcpy(kernel, gaussian_blur, sizeof(kernel));
	} else {
		printf("APPLY parameter invalid\n");
		return;
	}

	// Calculare sumei kernelului
	int kernel_sum = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			kernel_sum += kernel[i][j];
		}
	}
	if (kernel_sum == 0) kernel_sum = 1; // Pentru a nu imparti la 0

	// Alocare memorie
	unsigned char ***new_colors = malloc(img->height * sizeof(unsigned char **));
	for (int i = 0; i < img->height; i++) {
		new_colors[i] = malloc(img->width * sizeof(unsigned char *));
		for (int j = 0; j < img->width; j++) {
			new_colors[i][j] = malloc(3 * sizeof(unsigned char));
			for (int k = 0; k < 3; k++) {
				new_colors[i][j][k] = img->colors[i][j][k];
			}
		}
	}

	// Aplicare filtru
	for (int i = y1; i < y2; i++) {
		for (int j = x1; j < x2; j++) {
			for (int k = 0; k < 3; k++) { // Pe 
				if (i > 0 && i < img->height - 1 && j > 0 && j < img->width - 1) {
					int value = 0;
					for (int di = -1; di <= 1; di++) {
						for (int dj = -1; dj <= 1; dj++) {
							value += img->colors[i + di][j + dj][k] * kernel[di + 1][dj + 1];
						}
					}
					value /= kernel_sum;
					value = clamp(value, 0, 255);  // Limitarea valorii
					new_colors[i][j][k] = (unsigned char)value;
				}
			}
		}
	}

	// Eliberare memorie
	if (img->colors != NULL) {
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				free(img->colors[i][j]);
			}
			free(img->colors[i]);
		}
		free(img->colors);
	}

	// Actualizarea pozei
	img->colors = new_colors;
	printf("APPLY %s done\n", filter_name);
}

void save_image(Image *img, const char *filename, int is_ascii)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}

	FILE *file = fopen(filename, "w"); // Deschiderea fisierului
	if (!file) {
		printf("Failed to save %s\n", filename); // Mesaj eroare
		return;
	}

	// Scriem antetul
	if (img->is_color) {
		fprintf(file, is_ascii ? "P3\n" : "P6\n");
	} else {
		fprintf(file, is_ascii ? "P2\n" : "P5\n");
	}

	fprintf(file, "%d %d\n%d\n", img->width, img->height, img->max_value);

	// Afisare pixeli
	if (img->is_color) {
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				if (is_ascii) {
					// Scrierea valorilor RGB pentru fiecare pixel în format ascii P3
					fprintf(file, "%d %d %d ", 
							img->colors[i][j][0],
							img->colors[i][j][1],
							img->colors[i][j][2]);
				} else {
					// Scrierea valorilor RGB pentru fiecare pixel în format binar P6
					fwrite(&img->colors[i][j][0], 1, 1, file);
					fwrite(&img->colors[i][j][1], 1, 1, file);
					fwrite(&img->colors[i][j][2], 1, 1, file);
				}
			}
			if (is_ascii) {
				fprintf(file, "\n");
			}
		}
	} else {
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				if (is_ascii) {
					// Scrierea valorilor pentru fiecare pixel în format ascii P2
					fprintf(file, "%d ", img->pixels[i][j]);
				} else {
					// Scrierea valorilor pentru fiecare pixel în format binar P5
					fwrite(&img->pixels[i][j], 1, 1, file);
				}
			}
			if (is_ascii) {
				fprintf(file, "\n");
			}
		}
	}
	fclose(file);
	printf("Saved %s\n", filename);
}

void exit_program(Image *img)
{
	if (!img->is_loaded) {
		printf("No image loaded\n");
		return;
	}

	// Eliberare memorie pt grayscale
	if (!img->is_color) {
		for (int i = 0; i < img->height; i++) {
			free(img->pixels[i]);
		}
		free(img->pixels);
	} else {
		// Eliberare memorie pt color
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				free(img->colors[i][j]);
			}
			free(img->colors[i]);
		}
		free(img->colors);
	}

	img->is_loaded = 0; // Nu mai este incarcata imaginea
}

int main(void)
{
	Image img = {0}; // Imaginea
	char command[256]; // Char pentru comenzi
	// Facem selectia invalida initial
	img.selected_x1 = -1;
	img.selected_x2 = -1;
	img.selected_y1 = -1;
	img.selected_y2 = -1;

	while (1) {
		if (!fgets(command, sizeof(command), stdin)) {
			break;
		}
		command[strcspn(command, "\n")] = '\0'; // Fara newline
		// Apelarea comenzilor
		if (strncmp(command, "LOAD ", 5) == 0) {
			char filename[256];
			sscanf(command + 5, "%s", filename);
			load_image(&img, filename);
		} else if (strncmp(command, "SELECT ALL", 10) == 0) {
			select_all(&img);
		} else if (strncmp(command, "SELECT ", 7) == 0) {
			int x1, y1, x2, y2;
			if (sscanf(command + 7, "%d %d %d %d", &x1, &y1, &x2, &y2) == 4) {
				select_region(&img, x1, y1, x2, y2);
			} else {
				printf("Invalid command\n");
			}
		} else if (strncmp(command, "HISTOGRAM ", 10) == 0) {
			int x, y, z;
			if (sscanf(command + 10, "%d %d %d", &x, &y, &z) == 2) {
				histogram(&img, x, y);
			} else {
				// Daca are mai mult sau mai putin de 2 argumente
				printf("Invalid command\n");
			}
		} else if (strncmp(command, "HISTOGRAM", 9) == 0 && img.is_loaded != 1) {
			printf("No image loaded\n"); // Daca nu are argumente
		} else if (strncmp(command, "EQUALIZE", 8) == 0) {
			equalize(&img);
		} else if (strncmp(command, "ROTATE ", 7) == 0) {
			int angle;
			if (sscanf(command + 7, "%d", &angle) == 1) {
				rotate(&img, angle);
			} else {
				printf("Invalid command\n");
			}
		} else if (strncmp(command, "CROP", 4) == 0) {
			crop(&img);
		} else if (strncmp(command, "APPLY ", 6) == 0) {
			char parameter[256];
			sscanf(command + 6, "%s", parameter);
			apply_filter(&img, parameter);
		} else if (strncmp(command, "APPLY", 5) == 0 && img.is_loaded != 1) {
			printf("No image loaded\n"); // Daca nu are parametrii
		} else if (strncmp(command, "SAVE ", 5) == 0) {
			char filename[256], format[256] = "";
			int args = sscanf(command + 5, "%s %s", filename, format);
			if (args >= 1) { // Tipul de save
				save_image(&img, filename, strcmp(format, "ascii") == 0);
			} else {
				printf("Invalid command\n");
			}
		} else if (strncmp(command, "EXIT", 4) == 0) {
			exit_program(&img);
			break;
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}

