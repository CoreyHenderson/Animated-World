/// Loads an image file and returns the block of data in 32 bit format. Must be deleted by caller. 
char * loadPNG (const char * filename, int & width, int & height);

/// Loads several image files and packs them into a single texture from left to right.
char * loadAtlas (char * filenames [], int numfiles, int & width, int & height);

void savePNG (char * filename, char * data, int & width, int & height);