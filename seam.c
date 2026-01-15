#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int w, h;
    unsigned char *r, *g, *b;
} Image;

Image* load_ppm(const char *path) {
    FILE *f = fopen(path, "rb");
    char magic[3];
    int w, h, maxval;
    
    fscanf(f, "%2s %d %d %d", magic, &w, &h, &maxval);
    fgetc(f);
    
    Image *img = malloc(sizeof(Image));
    img->w = w;
    img->h = h;
    img->r = malloc(w * h);
    img->g = malloc(w * h);
    img->b = malloc(w * h);
    
    unsigned char *data = malloc(w * h * 3);
    fread(data, 1, w * h * 3, f);
    fclose(f);
    
    for (int i = 0; i < w * h; i++) {
        img->r[i] = data[i * 3];
        img->g[i] = data[i * 3 + 1];
        img->b[i] = data[i * 3 + 2];
    }
    
    free(data);
    return img;
}

void save_ppm(const char *path, Image *img) {
    FILE *f = fopen(path, "wb");
    fprintf(f, "P6\n%d %d\n255\n", img->w, img->h);
    
    for (int i = 0; i < img->w * img->h; i++) {
        fputc(img->r[i], f);
        fputc(img->g[i], f);
        fputc(img->b[i], f);
    }
    
    fclose(f);
}

float* compute_energy(Image *img) {
    float *energy = malloc(img->w * img->h * sizeof(float));
    
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            float dx = 0, dy = 0;
            
            if (x > 0 && x < img->w - 1) {
                int l = y * img->w + (x - 1);
                int r = y * img->w + (x + 1);
                dx = (img->r[r] - img->r[l]) + (img->g[r] - img->g[l]) + (img->b[r] - img->b[l]);
            }
            
            if (y > 0 && y < img->h - 1) {
                int u = (y - 1) * img->w + x;
                int d = (y + 1) * img->w + x;
                dy = (img->r[d] - img->r[u]) + (img->g[d] - img->g[u]) + (img->b[d] - img->b[u]);
            }
            
            energy[y * img->w + x] = sqrt(dx * dx + dy * dy);
        }
    }
    
    return energy;
}

int* find_seam(Image *img, float *energy) {
    float *dp = malloc(img->w * img->h * sizeof(float));
    int *parent = malloc(img->w * img->h * sizeof(int));
    
    for (int x = 0; x < img->w; x++) {
        dp[x] = energy[x];
        parent[x] = -1;
    }
    
    for (int y = 1; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            float min_val = 1e9;
            int min_x = x;
            
            for (int px = x - 1; px <= x + 1; px++) {
                if (px >= 0 && px < img->w) {
                    float val = dp[(y - 1) * img->w + px];
                    if (val < min_val) {
                        min_val = val;
                        min_x = px;
                    }
                }
            }
            
            dp[y * img->w + x] = min_val + energy[y * img->w + x];
            parent[y * img->w + x] = min_x;
        }
    }
    
    int min_x = 0;
    float min_val = dp[(img->h - 1) * img->w];
    for (int x = 1; x < img->w; x++) {
        float val = dp[(img->h - 1) * img->w + x];
        if (val < min_val) {
            min_val = val;
            min_x = x;
        }
    }
    
    int *seam = malloc(img->h * sizeof(int));
    seam[img->h - 1] = min_x;
    
    for (int y = img->h - 2; y >= 0; y--) {
        seam[y] = parent[(y + 1) * img->w + seam[y + 1]];
    }
    
    free(dp);
    free(parent);
    
    return seam;
}

Image* remove_seam(Image *img, int *seam) {
    Image *out = malloc(sizeof(Image));
    out->w = img->w - 1;
    out->h = img->h;
    out->r = malloc(out->w * out->h);
    out->g = malloc(out->w * out->h);
    out->b = malloc(out->w * out->h);
    
    for (int y = 0; y < img->h; y++) {
        int sx = seam[y];
        int out_x = 0;
        
        for (int x = 0; x < img->w; x++) {
            if (x != sx) {
                out->r[y * out->w + out_x] = img->r[y * img->w + x];
                out->g[y * out->w + out_x] = img->g[y * img->w + x];
                out->b[y * out->w + out_x] = img->b[y * img->w + x];
                out_x++;
            }
        }
    }
    
    return out;
}

void free_image(Image *img) {
    free(img->r);
    free(img->g);
    free(img->b);
    free(img);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <input.ppm> <output.ppm> <num_seams>\n", argv[0]);
        return 1;
    }
    
    const char *input_path = argv[1];
    const char *output_path = argv[2];
    int num_seams = atoi(argv[3]);
    
    Image *img = load_ppm(input_path);
    
    for (int i = 0; i < num_seams; i++) {
        float *energy = compute_energy(img);
        int *seam = find_seam(img, energy);
        
        Image *new_img = remove_seam(img, seam);
        
        free_image(img);
        free(energy);
        free(seam);
        
        img = new_img;
    }
    
    save_ppm(output_path, img);
    free_image(img);
    
    return 0;
}
