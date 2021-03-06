#include <stdio.h>
#include <string.h>//asda sdasd asd asd 
#include <math.h>
#include <CL/cl.h>//asdasdasd

cl_uint ObtenerPlataformas(cl_platform_id *&platform_ids, cl_uint &num_platforms) {
	cl_uint err, i;
	err=clGetPlatformIDs(0, NULL, &num_platforms);
	if (err!=CL_SUCCESS) {
		printf("Error al obtener el número de plataformas\n");
		return err;
	}
	//platform_ids=(cl_platform_id *)malloc(sizeof(cl_platform_id)*num_platforms);
	platform_ids=new cl_platform_id[num_platforms];
	err=clGetPlatformIDs(num_platforms, platform_ids, &num_platforms);
	if (err!=CL_SUCCESS) {
		printf("Error al obtener las plataformas\n");
		return err;
	}
	for (i=0; i<num_platforms; i++) {
		char nombre[1024];
		err=clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, 1024, nombre, NULL);
		if (err!=CL_SUCCESS) {
			printf("Error al obtener la información de la plataforma %d\n", i);
			return err;
		}
		printf("Plataforma %d:\t %s\n", i, nombre);
	}
	
	return CL_SUCCESS;
}

void LiberarPlataformas (cl_platform_id *&platform_ids) {
	free(platform_ids);
}

cl_uint ObtenerDispositivos(cl_platform_id platform_id, cl_device_type device_type, cl_device_id *&device_ids, cl_uint &num_devices) {
	cl_uint err, i;
	err=clGetDeviceIDs(platform_id, device_type, 0, NULL, &num_devices);
	if (err!=CL_SUCCESS) {
		printf("Error al obtener el número de dispositivos\n");
		return err;
	}
	//device_ids=(cl_device_id *)malloc(sizeof(cl_device_id)*num_devices);
	device_ids=new cl_device_id[num_devices];
	err=clGetDeviceIDs (platform_id, device_type, num_devices, device_ids, &num_devices);
	if (err!=CL_SUCCESS) {
		printf("Error al obtener los dispositivos\n");
		return err;
	}
	for (i=0; i<num_devices; i++) {
		char nombre[1024];
		err=clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, 1024, nombre, NULL);
		if (err!=CL_SUCCESS) {
			printf("Error al obtener la información de la plataforma %d\n", i);
			return err;
		}
		printf("Dispositivo %d:\t %s\n", i, nombre);
	}
	return CL_SUCCESS;
}

void LiberarDispositivos(cl_device_id *&device_ids) {
	free(device_ids);
}

cl_uint CrearContexto(cl_platform_id platform_id, cl_device_id *device_ids, cl_uint num_devices, cl_context &contexto) {
	cl_int err;
	cl_context_properties prop[3];
	prop[0]=CL_CONTEXT_PLATFORM;
	prop[1]=(cl_context_properties)platform_id;
	prop[2]=0;
	contexto = clCreateContext(prop, num_devices, device_ids, NULL, NULL ,&err);
	if (err!=CL_SUCCESS) {
		printf("Error en la creación del contexto\n");
		return err;
	}
	return CL_SUCCESS;
}

cl_uint CrearColas(cl_context contexto, cl_device_id device_id, cl_command_queue_properties que_prop, cl_uint que_size, cl_command_queue &cola) {
	cl_int num_prop=0, err;
	if (que_prop!=0) num_prop+=2;
	if (que_size!=0) num_prop+=2;
	cl_command_queue_properties prop_cola[num_prop];
	int desp=0;
	if (que_prop!=0) {
		prop_cola[0]=CL_QUEUE_PROPERTIES;
		prop_cola[1]=que_prop;
		desp=2;
	}
	if (que_size!=0) {
		prop_cola[desp]=CL_QUEUE_ON_DEVICE;
		prop_cola[desp+1]=(cl_command_queue_properties)que_size;
	}
	prop_cola[num_prop]=0;
	cola = clCreateCommandQueueWithProperties(contexto, device_id, prop_cola, &err);
	if (err!=CL_SUCCESS) {
		printf("Error en la creación de la cola\n");
		return err;
	}
	return CL_SUCCESS;
}

char leerFuentes(char *&src, const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (!file) {
        printf("Error al abrir el fichero '%s'\n", fileName);
        return 0;
    }

	if (fseek(file, 0, SEEK_END))  {
		printf("Error de posicionamiento en el '%s'\n", fileName);
		fclose(file);
		return 0;
    }

    long size = ftell(file);
    if (size == 0)  {
        printf("Error al calcular el tamaño del fichero '%s'\n", fileName);
        fclose(file);
        return 0;
    }

	rewind(file);
    src = (char *)malloc(sizeof(char) * size + 1);
    size_t res = fread(src, 1, sizeof(char) * size, file);
    if (res != sizeof(char) * size)   {
        printf("Error de lectura del fichero '%s'\n", fileName);
        fclose(file);
        free(src);
        return 0;
	}
    src[size] = '\0';
    fclose(file);
	return 1;
}

#define TAM 1000000

int main(int argc, char *argv[]){
	//ocl
	cl_platform_id *platform_ids;
	cl_uint num_platforms;
	cl_uint i,j;
  
	//Plataformas
	ObtenerPlataformas(platform_ids, num_platforms);
	
	//Dispositivos
	cl_uint num_devices;
	cl_device_id *device_ids;
	ObtenerDispositivos(platform_ids[0], CL_DEVICE_TYPE_ALL, device_ids, num_devices);

	//Contextos y colas
	cl_context contexto;
	cl_command_queue colas[num_devices];
	CrearContexto(platform_ids[0], device_ids, num_devices, contexto);
	//for (i=0; i<num_devices; i++)
		CrearColas(contexto, device_ids[0], CL_QUEUE_PROFILING_ENABLE, 0, colas[0]);
	
	//Programas y kernels
	//1.- Lectura del código fuente
	char *codigo;
	leerFuentes(codigo, "aaa");
	//leerFuentes(codigo[1], "MersenneTwister.h");
	
	//2. Creación del objeto programa
	cl_int errcode_ret;
	cl_program program=clCreateProgramWithSource(contexto, 1, (const char**)&codigo, NULL, &errcode_ret);

	//3. Compilación del programa
	if (clBuildProgram (program, 0, NULL, NULL, NULL, NULL)!=CL_SUCCESS) {
		//4. Comprobación de errores
		printf("Error de generación del programa:\n");
		char buffer[1024];
		clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, 1024, buffer, NULL);
		printf("%s\n", buffer);
	}
	
	//5. Creación del objeto kernel
	cl_kernel kernel = clCreateKernel (program, "mult_vec", &errcode_ret);
	if (errcode_ret!=CL_SUCCESS) {
		printf("Error en la generación del kernel\n");
		exit(1);
	}

	//Creación de buffers
	float vectorI[TAM], vectorO[TAM];
	for (i=0; i<TAM; i++)
		vectorI[i]=i;
	cl_mem bufferI=clCreateBuffer (contexto, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, TAM*sizeof(float), vectorI, &errcode_ret);
	if (errcode_ret!=CL_SUCCESS) {
		printf("Error en la creación del buffer de entrada\n");
		switch(errcode_ret) {
			case CL_INVALID_CONTEXT: printf("CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE : printf("CL_INVALID_VALUE \n"); break;
			case CL_INVALID_BUFFER_SIZE : printf("CL_INVALID_BUFFER_SIZE \n"); break;
			case CL_INVALID_HOST_PTR : printf("CL_INVALID_HOST_PTR \n"); break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE : printf("CL_MEM_OBJECT_ALLOCATION_FAILURE \n"); break;
			default: printf("CL_OUT_OF_RESOURCES");
		}
		exit(1);
	}
	cl_mem bufferO=clCreateBuffer (contexto, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, TAM*sizeof(float), vectorO, &errcode_ret);
	if (errcode_ret!=CL_SUCCESS) {
		printf("Error en la creación del buffer de salida\n");
		exit(1);
	}
	size_t tam=8;
	
	//Asignando los parámetros
	if (clSetKernelArg (kernel, 0, TAM*sizeof(float), &bufferI)!=CL_SUCCESS) {printf("error 1\n"); exit(1);}
	if (clSetKernelArg (kernel, 1, TAM*sizeof(float), &bufferO)!=CL_SUCCESS) {printf("error 2\n"); exit(1);}
	
	//Lanzando el kernel
	//clEnqueueWriteBuffer(colas[0], bufferI, CL_TRUE, 0, TAM*4, vectorI, 0, NULL, NULL);
	cl_event event;
	if (clEnqueueNDRangeKernel (colas[0], kernel, 1, NULL, &tam, NULL, 0, NULL, &event)!=CL_SUCCESS) {printf("error 3\n"); exit(1);}

	//clEnqueueReadBuffer(colas[0], bufferO, CL_TRUE, 0, TAM*4, vectorO, 0, NULL, NULL);
	clFinish(colas[0]);
	
	cl_ulong ini, fin;
	errcode_ret=clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ini, NULL);
	if (errcode_ret!=CL_SUCCESS) {
		switch(errcode_ret) {
			case CL_PROFILING_INFO_NOT_AVAILABLE: printf("CL_PROFILING_INFO_NOT_AVAILABLE\n"); break;
			case CL_INVALID_VALUE : printf("CL_INVALID_VALUE \n"); break;
			case CL_INVALID_EVENT : printf("CL_INVALID_EVENT \n"); break;
			case CL_OUT_OF_RESOURCES : printf("CL_OUT_OF_RESOURCES \n"); break;
			case CL_OUT_OF_HOST_MEMORY : printf("CL_OUT_OF_HOST_MEMORY \n"); break;
			default: printf("CL_OUT_OF_RESOURCES");
		}
		exit(1);
	}
	if (clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &fin, NULL)!=CL_SUCCESS) {printf("error 5\n");exit(1);}
	printf("%u ns\n", fin-ini);
/*	for (i=0; i<TAM; i++)
		printf ("%f ", vectorO[i]);
	printf("\n");*/






	cl_mem bufferI2=clCreateBuffer (contexto, CL_MEM_READ_ONLY, TAM*sizeof(float), NULL, &errcode_ret);
	if (errcode_ret!=CL_SUCCESS) {
		printf("Error en la creación del buffer de entrada\n");
		switch(errcode_ret) {
			case CL_INVALID_CONTEXT: printf("CL_INVALID_CONTEXT\n"); break;
			case CL_INVALID_VALUE : printf("CL_INVALID_VALUE \n"); break;
			case CL_INVALID_BUFFER_SIZE : printf("CL_INVALID_BUFFER_SIZE \n"); break;
			case CL_INVALID_HOST_PTR : printf("CL_INVALID_HOST_PTR \n"); break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE : printf("CL_MEM_OBJECT_ALLOCATION_FAILURE \n"); break;
			default: printf("CL_OUT_OF_RESOURCES");
		}
		exit(1);
	}
	cl_mem bufferO2=clCreateBuffer (contexto, CL_MEM_WRITE_ONLY, TAM*sizeof(float), NULL, &errcode_ret);
	if (errcode_ret!=CL_SUCCESS) {
		printf("Error en la creación del buffer de salida\n");
		exit(1);
	}
	
	//Asignando los parámetros
	if (clSetKernelArg (kernel, 0, TAM*sizeof(float), &bufferI2)!=CL_SUCCESS) {printf("error 1\n"); exit(1);}
	if (clSetKernelArg (kernel, 1, TAM*sizeof(float), &bufferO2)!=CL_SUCCESS) {printf("error 2\n"); exit(1);}
	
	//Lanzando el kernel
	clEnqueueWriteBuffer(colas[0], bufferI2, CL_TRUE, 0, TAM*sizeof(float), vectorI, 0, NULL, NULL);

	cl_event event2;
	if (clEnqueueNDRangeKernel (colas[0], kernel, 1, NULL, &tam, NULL, 0, NULL, &event2)!=CL_SUCCESS) {printf("error 3\n"); exit(1);}

	clEnqueueReadBuffer(colas[0], bufferO2, CL_TRUE, 0, TAM*sizeof(float), vectorO, 0, NULL, NULL);
	clFinish(colas[0]);
	
	cl_ulong ini2, fin2;
	if (clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ini2, NULL)!=CL_SUCCESS) {printf("error 4\n");exit(1);}
	if (clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &fin2, NULL)!=CL_SUCCESS) {printf("error 5\n");exit(1);}
	printf("%u ns\n", fin2-ini2);

	/*for (i=0; i<TAM; i++)
		printf ("%f ", vectorO[i]);
	printf("\n");*/
	
	return 0;
	
}
