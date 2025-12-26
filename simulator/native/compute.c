#include "init.h"

//Simulate it!
uint64_t simulate(
    const char *shader_src,
    const uint64_t shader_len,
    const uint32_t tree_grid_size,
    const uint8_t starting_factor,
    const bool print_enabled
) {
    uint64_t epoch_count = 0;
    
    //Device initialisation stuff...
    cl_device_id device = choose_device();

    //Device initialisation stuff...
    cl_platform_id platform;
    clGetDeviceInfo(
        device,
        CL_DEVICE_PLATFORM,
        sizeof(cl_platform_id),
        &platform,
        nullptr
    );

    size_t platform_name_len;
    clGetPlatformInfo(
        platform,
        CL_PLATFORM_NAME,
        0,
        nullptr,
        &platform_name_len
    );
    char platform_name[platform_name_len];
    clGetPlatformInfo(
        platform,
        CL_PLATFORM_NAME,
        platform_name_len,
        platform_name,
        nullptr
    );
    size_t device_name_len;
    clGetDeviceInfo(
        device,
        CL_DEVICE_NAME,
        0,
        nullptr,
        &device_name_len
    );
    char device_name[device_name_len];
    clGetDeviceInfo(
        device,
        CL_DEVICE_NAME,
        device_name_len,
        device_name,
        nullptr
    );
    printf("Simulating on: [%s] %s!\n\n", platform_name, device_name);

    //Device initialisation stuff...
    cl_context context = clCreateContext(
        nullptr,
        1,
        &device,
        nullptr,
        nullptr,
        nullptr
    );

    //Create the shader program
    cl_program shader = clCreateProgramWithSource(
        context,
        1,
        &shader_src,
        &shader_len,
        nullptr
    );

    //Compile the shader program
    clBuildProgram(
        shader,
        1,
        &device,
        "-cl-mad-enable -cl-std=CL2.0",
        nullptr,
        nullptr
    );

    cl_kernel shader_kernel = clCreateKernel(shader, "simulate", nullptr);

    cl_command_queue queue = clCreateCommandQueueWithProperties(
        context,
        device,
        nullptr,
        nullptr
    );

    uint64_t global_work_size[2] = { tree_grid_size, tree_grid_size };

    //Allocate Device memory and initialise it
    uint32_t* dimension = (uint32_t*) clSVMAlloc(
        context,
        CL_MEM_READ_ONLY,
        sizeof(uint32_t),
        0
    );
    clEnqueueSVMMap(
        queue,
        CL_TRUE,
        CL_MAP_WRITE,
        dimension,
        sizeof(uint32_t),
        0,
        nullptr,
        nullptr
    );
    *dimension = tree_grid_size;
    clEnqueueSVMUnmap(
        queue,
        dimension,
        0,
        nullptr,
        nullptr
    );

    uint64_t extended_grid_size =
        tree_grid_size * tree_grid_size + 4 * tree_grid_size + 4;

    TEMP = malloc(extended_grid_size);

    uint8_t* tree_grid = (uint8_t*) clSVMAlloc(
        context,
        CL_MEM_READ_WRITE,
        sizeof(uint8_t) * extended_grid_size,
        0
    );
    initialize_trees(
        queue,
        tree_grid,
        extended_grid_size,
        tree_grid_size,
        starting_factor,
        print_enabled
    );

    uint8_t* randoms = (uint8_t*) clSVMAlloc(
        context,
        CL_MEM_READ_ONLY,
        sizeof(uint8_t) * tree_grid_size * tree_grid_size,
        0
    );
    uint8_t* next_tree_grid = (uint8_t*) clSVMAlloc(
        context,
        CL_MEM_READ_WRITE,
        sizeof(uint8_t) * extended_grid_size,
        0
    );
    for (size_t i = 0; i < extended_grid_size; i++)
        TEMP[i] = 3;
    clEnqueueSVMMemcpy(
        queue,
        CL_FALSE,
        next_tree_grid,
        TEMP,
        extended_grid_size,
        0,
        nullptr,
        nullptr
    );

    uint32_t* done = (uint32_t*) clSVMAlloc(
        context,
        CL_MEM_READ_WRITE,
        sizeof(uint32_t),
        0
    );

    clSetKernelArgSVMPointer(shader_kernel, 0, dimension);
    clSetKernelArgSVMPointer(shader_kernel, 2, randoms);
    clSetKernelArgSVMPointer(shader_kernel, 4, done);
    //Initialisation finished

    //Start the timer...
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    uint64_t t1 = (
        (uint64_t) time.tv_sec * (uint64_t) 1'000'000'000 +
        (uint64_t) time.tv_nsec
    );

    //Cuz the fun begins!
    do {
        //TFF magic
        clSetKernelArgSVMPointer(
            shader_kernel,
            1,
            get_tree_grid(tree_grid, next_tree_grid)
        );
        clSetKernelArgSVMPointer(
            shader_kernel,
            3,
            get_next_tree_grid(tree_grid, next_tree_grid)
        );

        reset_done(queue, done);
        get_randoms(queue, randoms, tree_grid_size * tree_grid_size);

        //Start computing on the Device!!!!
        clEnqueueNDRangeKernel(
            queue,
            shader_kernel,
            2,
            nullptr,
            global_work_size,
            nullptr,
            0,
            nullptr,
            nullptr
        );

        if (print_enabled) {
            clEnqueueSVMMap(
                queue,
                CL_TRUE,
                CL_MAP_READ,
                get_next_tree_grid(tree_grid, next_tree_grid),
                extended_grid_size,
                0,
                nullptr,
                nullptr
            );
            for (size_t j = 0; j < tree_grid_size; j++) {
                for (size_t i = 0; i < tree_grid_size; i++) {
                    printf(
                        "%d",
                        get_next_tree_grid(
                            tree_grid,
                            next_tree_grid
                        )[(j + 1) * (tree_grid_size + 2) + i + 1]);
                }
                printf("\n");
            }
            clEnqueueSVMUnmap(
                queue,
                get_next_tree_grid(tree_grid, next_tree_grid),
                0,
                nullptr,
                nullptr
            );
            printf("\n");
        }
        clFlush(queue);
        clFinish(queue);

        //Toggle,
        TFF = TFF ^ 1;
        //and increment!
        epoch_count += 1;
    } while (!is_done(queue, done));

    //Stop the timer! We're done!
    clock_gettime(CLOCK_MONOTONIC, &time);
    uint64_t t2 = (
        (uint64_t) time.tv_sec * (uint64_t) 1'000'000'000 +
        (uint64_t) time.tv_nsec
    );
    printf("Simulation finished in: %lums.\n", (t2 - t1) / 1'000'000);

    printf("\nStats:\n");
    printf("Initial: Alive: %lu, Burning: %lu, Dead: 0\n", STATS[0], STATS[1]);
    STATS[0] = 0;
    STATS[1] = 0;

    clEnqueueSVMMap(
        queue,
        CL_TRUE,
        CL_MAP_READ,
        get_tree_grid(tree_grid, next_tree_grid),
        extended_grid_size,
        0,
        nullptr,
        nullptr
    );
    for (size_t j = 0; j < tree_grid_size; j++) {
        for (size_t i = 0; i < tree_grid_size; i++) {
            if (
                get_tree_grid(
                    tree_grid,
                    next_tree_grid
                )[(j + 1) * (tree_grid_size + 2) + i + 1] == 1
            )
                STATS[0] += 1;
            else if (
                get_tree_grid(
                    tree_grid,
                    next_tree_grid
                )[(j + 1) * (tree_grid_size + 2) + i + 1] == 3
            )
                STATS[2] += 1;
        }
    }
    clEnqueueSVMUnmap(
        queue,
        get_tree_grid(tree_grid, next_tree_grid),
        0,
        nullptr,
        nullptr
    );

    printf("Final  : Alive: %lu, Burning: 0, Dead: %lu\n\n", STATS[0], STATS[2]);

    clFlush(queue);
    clFinish(queue);

    //Usual deallocation
    clSVMFree(context, done);
    clSVMFree(context, next_tree_grid);
    clSVMFree(context, randoms);
    clSVMFree(context, tree_grid);
    clSVMFree(context, dimension);

    free(TEMP);

    clReleaseCommandQueue(queue);
    clReleaseKernel(shader_kernel);
    clReleaseProgram(shader);
    clReleaseContext(context);
    clReleaseDevice(device);

    //Result!!!
    return epoch_count;
}