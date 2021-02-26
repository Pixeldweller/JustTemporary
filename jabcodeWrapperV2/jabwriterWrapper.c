#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jabcode.h"

jab_data* 		data = 0;
jab_char* 		filename = 0;
jab_int32 		color_number = 0;
jab_int32 		symbol_number = 0;
jab_int32 		module_size = 0;
jab_int32		master_symbol_width = 0;
jab_int32 		master_symbol_height= 0;
jab_int32* 		symbol_positions = 0;
jab_int32 		symbol_positions_number = 0;
jab_vector2d* 	symbol_versions = 0;
jab_int32 		symbol_versions_number = 0;
jab_int32* 		symbol_ecc_levels = 0;
jab_int32 		symbol_ecc_levels_number = 0;
jab_int32		color_space = 0;

struct jabDTO {
   char* inputstring;
   char* filename;
   int	color_number;
   int 		symbol_number;
   int 		module_size;
   int		master_symbol_width;
   int 		master_symbol_height;
   char* 		symbol_positions;
   int 		symbol_positions_number;
   char* symbol_versions;
   //jab_vector2d* 	symbol_versions;
   int 		symbol_versions_number;
   char* 		symbol_ecc_levels;
   int 		symbol_ecc_levels_number;
   int		color_space;
};

struct jab_bitmap_mod{
   int	width;
   int	height;
   int	bits_per_pixel;
   int	bits_per_channel;
   int	channel_count;
   char* pixel;
};

void reportErrorMod(const char* message){
	  int msgboxID = MessageBox(
        NULL,
        message,
        "Debug",
        MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
    );
}

/**
 * @brief Print usage of JABCode writer
*/
void printUsage()
{
	printf("\n");
	printf("jabcodeWriter (Version %s Build date: %s) - Fraunhofer SIT\n\n", VERSION, BUILD_DATE);
	printf("Usage:\n\n");
	printf("jabcodeWriter --input message-to-encode --output output-image [options]\n");
	printf("\n");
	printf("--input\t\t\tInput data (message to be encoded).\n");
	printf("--input-file\t\tInput data file.\n");
    printf("--output\t\tOutput image file.\n");
    printf("--color-number\t\tNumber of colors (4,8,default:8).\n");
	printf("--module-size\t\tModule size in pixel (default:12 pixels).\n");
    printf("--symbol-width\t\tMaster symbol width in pixel.\n");
    printf("--symbol-height\t\tMaster symbol height in pixel.\n");
	printf("--symbol-number\t\tNumber of symbols (1-61, default:1).\n");
    printf("--ecc-level\t\tError correction levels (1-10, default:3(6%%)). If\n\t\t\t"
						  "different for each symbol, starting from master and\n\t\t\t"
						  "then slave symbols (ecc0 ecc1 ecc2...). For master\n\t\t\t"
						  "symbol, level 0 means using the default level, for\n\t\t\t"
						  "slaves, it means using the same level as its host.\n");
    printf("--symbol-version\tSide-Version of each symbol, starting from master and\n\t\t\t"
							 "then slave symbols (x0 y0 x1 y1 x2 y2...).\n");
    printf("--symbol-position\tSymbol positions (0-60), starting from master and\n\t\t\t"
							  "then slave symbols (p0 p1 p2...). Only required for\n\t\t\t"
							  "multi-symbol code.\n");
	printf("--color-space\t\tColor space of output image (0:RGB,1:CMYK,default:0).\n\t\t\t"
							"RGB image is saved as PNG and CMYK image as TIFF.\n");
    printf("--help\t\t\tPrint this help.\n");
    printf("\n");
    printf("Example for 1-symbol-code: \n");
    printf("jabcodeWriter --input 'Hello world' --output test.png\n");
    printf("\n");
    printf("Example for 3-symbol-code: \n" );
    printf("jabcodeWriter --input 'Hello world' --output test.png --symbol-number 3 --symbol-position 0 3 2 --symbol-version 3 2 4 2 3 2\n");
    printf("\n");
}


/**
 * @brief Parse command line parameters
 * @return 1: success | 0: failure
*/
jab_boolean parseCommandLineParameters(jab_int32 para_number, jab_char* para[])
{
	//first scan
	for (jab_int32 loop=1; loop<para_number; loop++)
	{
		if (0 == strcmp(para[loop],"--input"))
        {
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", para[loop]);
				return 0;
			}
            jab_char* data_string = para[++loop];
            if(data) free(data);
            data = (jab_data *)malloc(sizeof(jab_data) + strlen(data_string) * sizeof(jab_char));
            if(!data)
            {
                reportError("Memory allocation for input data failed");
                return 0;
            }
			data->length = strlen(data_string);
			memcpy(data->data, data_string, strlen(data_string));
        }
        else if (0 == strcmp(para[loop],"--input-file"))
        {
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", para[loop]);
				return 0;
			}
            FILE* fp = fopen(para[++loop], "rb");
            if(!fp)
            {
				reportError("Opening input data file failed");
                return 0;
            }
			jab_int32 file_size;
			fseek(fp, 0, SEEK_END);
			file_size = ftell(fp);
			if(data) free(data);
            data = (jab_data *)malloc(sizeof(jab_data) + file_size * sizeof(jab_char));
            if(!data)
            {
                reportError("Memory allocation for input data failed");
                return 0;
            }
            fseek(fp, 0, SEEK_SET);
            if(fread(data->data, 1, file_size, fp) != file_size)
            {
				reportError("Reading input data file failed");
				free(data);
				fclose(fp);
				return 0;
			}
			fclose(fp);
			data->length = file_size;
        }
		else if (0 == strcmp(para[loop],"--output"))
        {
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", para[loop]);
				return 0;
			}
            filename = para[++loop];
        }
        else if (0 == strcmp(para[loop],"--color-number"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            char* endptr;
			color_number = strtol(para[++loop], &endptr, 10);
			if(*endptr)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
            if(color_number != 4  && color_number != 8)
            {
				reportError("Invalid color number. Supported color number includes 4 and 8.");
				return 0;
            }
        }
        else if (0 == strcmp(para[loop],"--module-size"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            char* endptr;
			module_size = strtol(para[++loop], &endptr, 10);
			if(*endptr || module_size < 0)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
        }
        else if (0 == strcmp(para[loop],"--symbol-width"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            char* endptr;
			master_symbol_width = strtol(para[++loop], &endptr, 10);
			if(*endptr || master_symbol_width < 0)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
        }
        else if (0 == strcmp(para[loop],"--symbol-height"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            char* endptr;
			master_symbol_height = strtol(para[++loop], &endptr, 10);
			if(*endptr || master_symbol_height < 0)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
        }
        else if (0 == strcmp(para[loop],"--symbol-number"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
                return 0;
			}
			char* endptr;
			symbol_number = strtol(para[++loop], &endptr, 10);
            if(*endptr)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
            if(symbol_number < 1 || symbol_number > MAX_SYMBOL_NUMBER)
            {
				reportError("Invalid symbol number (must be 1 - 61).");
				return 0;
            }
        }
		else if (0 == strcmp(para[loop],"--color-space"))
		{
        	char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
                return 0;
			}
			char* endptr;
			color_space = strtol(para[++loop], &endptr, 10);
            if(*endptr)
			{
				printf("Invalid or missing values for option '%s'.\n", option);
				return 0;
			}
            if(color_space !=0 && color_space != 1)
            {
				reportError("Invalid color space (must be 0 or 1).");
				return 0;
            }
        }
	}

	//check input
    if(!data)
    {
		reportError("Input data missing");
		return 0;
    }
    else if(data->length == 0)
    {
		reportError("Input data is empty");
		return 0;
    }
    if(!filename)
    {
		reportError("Output file missing");
		return 0;
    }
    if(symbol_number == 0)
    {
		symbol_number = 1;
    }

	//second scan
    for (jab_int32 loop=1; loop<para_number; loop++)
    {
        if (0 == strcmp(para[loop],"--ecc-level"))
        {
			char* option = para[loop];
            if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            symbol_ecc_levels = (jab_int32 *)calloc(symbol_number, sizeof(jab_int32));
            if(!symbol_ecc_levels)
            {
                reportError("Memory allocation for symbol ecc levels failed");
                return 0;
            }
            for (jab_int32 j=0; j<symbol_number; j++)
            {
				if(loop + 1 > para_number - 1)
					break;
				char* endptr;
				symbol_ecc_levels[j] = strtol(para[++loop], &endptr, 10);
				if(*endptr)
				{
					if(symbol_ecc_levels_number == 0)
					{
						printf("Value for option '%s' missing or invalid.\n", option);
						return 0;
					}
					loop--;
					break;
				}
                if(symbol_ecc_levels[j] < 0 || symbol_ecc_levels[j] > 10)
				{
                    reportError("Invalid error correction level (must be 1 - 10).");
					return 0;
				}
				symbol_ecc_levels_number++;
			}
        }
        else if (0 == strcmp(para[loop],"--symbol-version"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            symbol_versions = (jab_vector2d *)calloc(symbol_number, sizeof(jab_vector2d));
            if(!symbol_versions)
            {
                reportError("Memory allocation for symbol versions failed");
                return 0;
            }
			for(jab_int32 j=0; j<symbol_number; j++)
			{
				if(loop + 1 > para_number - 1)
				{
					printf("Too few values for option '%s'.\n", option);
					return 0;
				}
				char* endptr;
				symbol_versions[j].x = strtol(para[++loop], &endptr, 10);
				if(*endptr)
				{
					printf("Invalid or missing values for option '%s'.\n", option);
					return 0;
				}
				if(loop + 1 > para_number - 1)
				{
					printf("Too few values for option '%s'.\n", option);
					return 0;
				}
				symbol_versions[j].y = strtol(para[++loop], &endptr, 10);
				if(*endptr)
				{
					printf("Invalid or missing values for option '%s'.\n", option);
					return 0;
				}
				if(symbol_versions[j].x < 1 || symbol_versions[j].x > 32 || symbol_versions[j].y < 1 || symbol_versions[j].y > 32)
				{
					reportError("Invalid symbol side version (must be 1 - 32).");
					return 0;
				}
				symbol_versions_number++;
			}
        }
        else if (0 == strcmp(para[loop],"--symbol-position"))
        {
			char* option = para[loop];
			if(loop + 1 > para_number - 1)
			{
				printf("Value for option '%s' missing.\n", option);
				return 0;
			}
            symbol_positions = (jab_int32 *)calloc(symbol_number, sizeof(jab_int32));
            if(!symbol_positions)
            {
                reportError("Memory allocation for symbol positions failed");
                return 0;
            }
            for(jab_int32 j=0; j<symbol_number; j++)
            {
				if(loop + 1 > para_number - 1)
				{
					printf("Too few values for option '%s'.\n", option);
					return 0;
				}
				char* endptr;
				symbol_positions[j] = strtol(para[++loop], &endptr, 10);
				if(*endptr)
				{
					printf("Invalid or missing values for option '%s'.\n", option);
					return 0;
				}
				if(symbol_positions[j] < 0 || symbol_positions[j] > 60)
				{
					reportError("Invalid symbol position value (must be 0 - 60).");
					return 0;
				}
				symbol_positions_number++;
            }
        }
    }

    //check input
    if(symbol_number == 1 && symbol_positions)
    {
		if(symbol_positions[0] != 0)
		{
			reportError("Incorrect symbol position value for master symbol.");
			return 0;
		}
    }
	if(symbol_number > 1 && symbol_positions_number != symbol_number)
    {
        reportError("Symbol position information is incomplete for multi-symbol code");
        return 0;
    }
    if (symbol_number > 1 && symbol_versions_number != symbol_number)
    {
		reportError("Symbol version information is incomplete for multi-symbol code");
        return 0;
    }
    return 1;
}

jab_boolean parseDTOParameters(struct jabDTO* ptrDTO)
{
	jab_char* data_string = ptrDTO->inputstring;
	if(data) free(data);
	data = (jab_data *)malloc(sizeof(jab_data) + strlen(data_string) * sizeof(jab_char));
	if(!data)
	{
		reportError("Memory allocation for input data failed");
		return 0;
	}
	data->length = strlen(data_string);
	memcpy(data->data, data_string, strlen(data_string));
	
	filename = ptrDTO->filename;
	color_number = ptrDTO->color_number;
	symbol_number = ptrDTO->symbol_number;
	module_size = ptrDTO->module_size;
	master_symbol_width = ptrDTO->master_symbol_width;
	master_symbol_height = ptrDTO->master_symbol_height;
	//symbol_versions = {ptrDTO->symbol_versions_x, ptrDTO->symbol_versions_y}
	//symbol_positions = ptrDTO->symbol_positions;
	symbol_positions_number = ptrDTO->symbol_positions_number;
	//symbol_ecc_levels = ptrDTO->symbol_ecc_levels;
	color_space = ptrDTO->color_space;
	

	//check input
    if(!data)
    {
		reportError("Input data missing");
		return 0;
    }
    else if(data->length == 0)
    {
		reportError("Input data is empty");
		return 0;
    }
    if(!filename)
    {
		reportError("Output file nicht angegeben -> write to %temp%");
		//return 0;
    }
    if(symbol_number == 0)
    {
		symbol_number = 1;
    }	
  
	
	if(ptrDTO->symbol_ecc_levels)
	{
		symbol_ecc_levels = (jab_int32 *)calloc(symbol_number, sizeof(jab_int32));
		if(!symbol_ecc_levels)
		{
			reportErrorMod("Memory allocation for symbol ecc levels failed");
			return 0;
		}
		int i = 0;

		char *copy =  malloc(strlen(ptrDTO->symbol_ecc_levels));		
		strcpy(copy, ptrDTO->symbol_ecc_levels); 	
		
		char *end = copy;
		while(*end) {
			int n = strtol(copy, &end, 10);
			printf("%d\n", n);
			symbol_ecc_levels[i] = n;
			i++;
			while (*end == ' ') {
				end++;
			}
			copy = end;
		}
		
		for (jab_int32 j=0; j<symbol_number; j++)
		{
			if(symbol_ecc_levels[j] < 0 || symbol_ecc_levels[j] > 10)
			{
				reportErrorMod("Invalid error correction level (must be 1 - 10).");
				return 0;
			}
			symbol_ecc_levels_number++;
		}
	}
	if(ptrDTO->symbol_versions)
	{
		symbol_versions = (jab_vector2d *)calloc(symbol_number, sizeof(jab_vector2d));
		if(!symbol_versions)
		{
			reportErrorMod("Memory allocation for symbol versions failed");
			return 0;
		}
		int i = 0;
		char *copy =  malloc(strlen(ptrDTO->symbol_versions));		
		strcpy(copy, ptrDTO->symbol_versions); 	
		char *end = copy;
		while(*end) {
			int n = strtol(copy, &end, 10);
			//printf("%d\n", n);
			reportErrorMod(copy);
			symbol_versions[i].x = n;			
			while (*end == ' ') {
				end++;
			}
			copy = end;
			
			int m = strtol(copy, &end, 10);
			//printf("%d\n", m);
			symbol_versions[i].y = m;
			i++;
			while (*end == ' ') {
				end++;
			}
			copy = end;
		}
		
		for(jab_int32 j=0; j<symbol_number; j++)
		{			
			if(symbol_versions[j].x < 1 || symbol_versions[j].x > 32 || symbol_versions[j].y < 1 || symbol_versions[j].y > 32)
			{
				char* numberstring = malloc(20);
				sprintf(numberstring, "%d|%d", symbol_versions[j].x,symbol_versions[j].y);
				reportErrorMod(numberstring);
				reportErrorMod("Invalid symbol side version (must be 1 - 32).");
				return 0;
			}
			symbol_versions_number++;
		}
	}
	if(ptrDTO->symbol_positions)
	{
		symbol_positions = (jab_int32 *)calloc(symbol_number, sizeof(jab_int32));
            if(!symbol_positions)
            {
                reportErrorMod("Memory allocation for symbol positions failed");
                return 0;
            }
			int i = 0;
			char *copy =  malloc(strlen(ptrDTO->symbol_positions));		
			strcpy(copy, ptrDTO->symbol_positions); 	
			
			char *end = copy;
			while(*end) {
				int n = strtol(copy, &end, 10);
				printf("%d\n", n);
				symbol_positions[i] = n;
				i++;
				while (*end == ' ') {
					end++;
				}
				copy = end;
			}
			
            for(jab_int32 j=0; j<symbol_number; j++)
            {				
				if(symbol_positions[j] < 0 || symbol_positions[j] > 60)
				{
					reportErrorMod("Invalid symbol position value (must be 0 - 60).");
					return 0;
				}
				symbol_positions_number++;
            }
	}
	
	  //check input
    if(symbol_number == 1 && symbol_positions)
    {
		if(symbol_positions[0] != 0)
		{
			reportErrorMod("Incorrect symbol position value for master symbol.");
			return 0;
		}
    }
    if (symbol_number > 1 && symbol_versions_number != symbol_number)
    {
		reportErrorMod("Symbol version information is incomplete for multi-symbol code");
        return 0;
    }
	
    return 1;
}

/**
 * @brief Free allocated buffers
*/
void cleanMemory()
{
	free(data);
	free(symbol_positions);
	free(symbol_versions);
	free(symbol_ecc_levels);
}

/**
 * @brief JABCode writer main function
 * @return 0: success | 1: failure
*/
int main(int argc, char *argv[])
{
    if(argc < 2 || (0 == strcmp(argv[1],"--help")))
	{
		printUsage();
		return 1;
	}
	if(!parseCommandLineParameters(argc, argv))
	{
		return 1;
	}

    //create encode parameter object
    jab_encode* enc = createEncode(color_number, symbol_number);
    if(enc == NULL)
    {
		cleanMemory();
		reportError("Creating encode parameter failed");
        return 1;
    }
    if(module_size > 0)
    {
		enc->module_size = module_size;
    }
    if(master_symbol_width > 0)
    {
		enc->master_symbol_width = master_symbol_width;
    }
    if(master_symbol_height > 0)
    {
		enc->master_symbol_height = master_symbol_height;
    }
	for(jab_int32 loop=0; loop<symbol_number; loop++)
	{
		if(symbol_ecc_levels)
			enc->symbol_ecc_levels[loop] = symbol_ecc_levels[loop];
		if(symbol_versions)
			enc->symbol_versions[loop] = symbol_versions[loop];
		if(symbol_positions)
			enc->symbol_positions[loop] = symbol_positions[loop];
	}

	//generate JABCode
	if(generateJABCode(enc, data) != 0)
	{
		reportError("Creating jab code failed");
		destroyEncode(enc);
		cleanMemory();
		return 1;
	}

	//save bitmap in image file
	jab_int32 result = 0;
	if(color_space == 0)
	{
		if(!saveImage(enc->bitmap, filename))
		{
			reportError("Saving png image failed");
			result = 1;
		}
	}
	else if(color_space == 1)
	{
		if(!saveImageCMYK(enc->bitmap, 0, filename))
		{
			reportError("Saving tiff image failed");
			result = 1;
		}
	}

	destroyEncode(enc);
	cleanMemory();
	return result;
}



__declspec(dllexport) const char* __stdcall writeToTmpFile(struct jabDTO* ptrDTO) {
	
	//Debug
	//reportErrorMod(ptrDTO->inputstring);
		
	if(!parseDTOParameters(ptrDTO))
	{
		reportErrorMod("Parameter inkorrekt");
	}
	
	if(!ptrDTO->filename){
		const char* name = "\\tmpjabcode.png";
		const char* path = getenv( "TEMP" );;

		free(filename);
		filename = malloc(strlen(path)+strlen(name)); /* make space for the new string (should check the return value ...) */
		strcpy(filename, path); /* copy name into the new var */
		strcat(filename, name); /* add the extension */
	}
	
	jab_encode* enc = createEncode(color_number, symbol_number);
    if(enc == NULL)
    {
		cleanMemory();
		reportErrorMod("Creating encode parameter failed -> Encoding kombination ungültig");
        return "fehler";
    }
	
	int number = getSymbolCapacity(enc, 0);	
	char numberstring[10];
	sprintf(numberstring, "%d", number);
	//reportErrorMod(numberstring);
	
    if(module_size > 0)
    {
		enc->module_size = module_size;
    }
    if(master_symbol_width > 0)
    {
		enc->master_symbol_width = master_symbol_width;
    }
    if(master_symbol_height > 0)
    {
		enc->master_symbol_height = master_symbol_height;
    }
	for(jab_int32 loop=0; loop<symbol_number; loop++)
	{
		if(symbol_ecc_levels)
			enc->symbol_ecc_levels[loop] = symbol_ecc_levels[loop];
		if(symbol_versions)
			enc->symbol_versions[loop] = symbol_versions[loop];
		if(symbol_positions)
			enc->symbol_positions[loop] = symbol_positions[loop];
	}

	//generate JABCode
	if(generateJABCode(enc, data) != 0)
	{
		reportErrorMod("Jabcode Generator Fehler -> Parameter kombination ungültig!");
		destroyEncode(enc);
		cleanMemory();
		//return 1;
		return "fehler";
	}

	//save bitmap in image file
	jab_int32 result = 0;
	if(color_space == 0)
	{
		if(!saveImage(enc->bitmap, filename))
		{
			reportErrorMod("Saving png image failed");
			//result = 1;
		}
	}
	else if(color_space == 1)
	{
		if(!saveImageCMYK(enc->bitmap, 0, filename))
		{
			reportErrorMod("Saving tiff image failed");
			//result = 1;
		}
	}

	destroyEncode(enc);
	cleanMemory();
	return filename;
}

// Causes still causes Word problem
__declspec(dllexport) const struct jab_bitmap_mod* __stdcall writeToByteArray(struct jabDTO* ptrDTO, struct jab_bitmap_mod* ptrBitmap) {
	
	//Debug
	reportErrorMod(ptrDTO->inputstring);
		
	if(!parseDTOParameters(ptrDTO))
	{
		reportErrorMod("Parameter inkorrekt");
	}
	
	if(!ptrDTO->filename){
		const char* name = "\\tmpjabcode.png";
		const char* path = getenv( "TEMP" );;

		free(filename);
		filename = malloc(strlen(path)+strlen(name)); /* make space for the new string (should check the return value ...) */
		strcpy(filename, path); /* copy name into the new var */
		strcat(filename, name); /* add the extension */
	}
	
	jab_encode* enc = createEncode(color_number, symbol_number);
    if(enc == NULL)
    {
		cleanMemory();
		reportErrorMod("Creating encode parameter failed");
        return NULL;
    }
    if(module_size > 0)
    {
		enc->module_size = module_size;
    }
    if(master_symbol_width > 0)
    {
		enc->master_symbol_width = master_symbol_width;
    }
    if(master_symbol_height > 0)
    {
		enc->master_symbol_height = master_symbol_height;
    }
	for(jab_int32 loop=0; loop<symbol_number; loop++)
	{
		if(symbol_ecc_levels)
			enc->symbol_ecc_levels[loop] = symbol_ecc_levels[loop];
		if(symbol_versions)
			enc->symbol_versions[loop] = symbol_versions[loop];
		if(symbol_positions)
			enc->symbol_positions[loop] = symbol_positions[loop];
	}

	
	
	//generate JABCode
	if(generateJABCode(enc, data) != 0)
	{
		reportErrorMod("Creating jab code failed");
		destroyEncode(enc);
		cleanMemory();
		//return 1;
	}


	//destroyEncode(enc);
	cleanMemory();
	ptrBitmap->width=enc->bitmap->width;
	ptrBitmap->height=enc->bitmap->height;
	ptrBitmap->channel_count=4;
	ptrBitmap->pixel=enc->bitmap->pixel;
	return ptrBitmap;
}



__declspec(dllexport) const char* __stdcall readFromJabPNG(struct jabDTO* ptrDTO)
{


	jab_boolean output_as_file = 0;	

	//load image
	jab_bitmap* bitmap;
	bitmap = readImage("C:\\Users\\fabio\\Desktop\\jabcode-master\\dll_test\\jabcodeWrapper\\test.png");
	if(bitmap == NULL)
		return "error#1 - JAB Datei nicht vorhanden oder kein .png";

	//find and decode JABCode in the image
	jab_int32 decode_status;
	jab_decoded_symbol symbols[MAX_SYMBOL_NUMBER];
	jab_data* decoded_data = decodeJABCodeEx(bitmap, NORMAL_DECODE, &decode_status, symbols, MAX_SYMBOL_NUMBER);
	if(decoded_data == NULL)
	{
		free(bitmap);
		reportError("Decoding JABCode failed");
		if(decode_status > 0)
			return "error#2 - Dekodierungsfehler -> nur halb dekodierbar";
		else			
			return "error#3 - Dekodierungsfehler -> 4 Colormode Fallback möglich";
	}

	//output warning if the code is only partly decoded with COMPATIBLE_DECODE mode
	if(decode_status == 2)
	{
		JAB_REPORT_INFO(("The code is only partly decoded. Some slave symbols have not been decoded and are ignored."))
	}

	char * result;
	int i;
	
	result = malloc(decoded_data->length);
	for(i=0; i<decoded_data->length; i++)
		printf("%c", decoded_data->data[i]);
		result[i] = decoded_data->data[i];
	printf("\n");



	result = malloc(decoded_data->length); /* make space for the new string (should check the return value ...) */
	strcpy(result, decoded_data->data); /* copy name into the new var */
	
	free(bitmap);
	free(decoded_data);
    return result;
}

__declspec(dllexport) const char* __stdcall getMasterSymbolEncodingCapacity(struct jabDTO* ptrDTO)
{
	if(!parseDTOParameters(ptrDTO))
	{
		reportErrorMod("Encoding Parameter für Jabcode invalide...");
	}
	
	jab_encode* enc = createEncode(color_number, symbol_number);
    if(enc == NULL)
    {
		cleanMemory();
		reportErrorMod("Creating encode parameter failed");
        return "-1";
    }
	
	int number = getSymbolCapacity(enc, 0);	
	char* numberstring = malloc(10);
	sprintf(numberstring, "%d", number);
	
	return numberstring;
}

// Referenz zum speichern der Bitmap:
/*

jab_boolean saveImage(jab_bitmap* bitmap, jab_char* filename)
{
	png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if(bitmap->channel_count == 4)
    {
		image.format = PNG_FORMAT_RGBA;
		image.flags  = PNG_FORMAT_FLAG_ALPHA | PNG_FORMAT_FLAG_COLOR;
    }
    else
    {
		image.format = PNG_FORMAT_GRAY;
    }

    image.width  = bitmap->width;
    image.height = bitmap->height;

    if (png_image_write_to_file(&image,
								filename,
								0,
								bitmap->pixel,
								0,
								NULL) == 0)
	{
		reportError(image.message);
		reportError("Saving png image failed");
		return JAB_FAILURE;
	}
	return JAB_SUCCESS;
}
*/

