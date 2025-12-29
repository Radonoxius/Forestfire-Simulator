#pragma once

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <CL/cl.h>

//Device could be GPU, CPU, NPU, anything!
//Holds temporary values to be sent to Device...
uint8_t* TEMP;

//A T-Flip-Flop initialized to 0?
uint8_t TFF = 0;

//Forest stats: Live, Burning and Dead
uint64_t STATS[3] = { 0, 0, 0 };

#ifdef _WIN64

#include <windows.h>
#include <bcrypt.h>
#include <ntstatus.h>

void generate_randoms(size_t len) {
    NTSTATUS status;

    status = BCryptGenRandom(
        NULL,
        (PUCHAR)TEMP,
        (ULONG)len,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );

    if (status != STATUS_SUCCESS) {
        fprintf(stderr,
                "BCryptGenRandom failed (NTSTATUS = 0x%08lx)\n",
                (unsigned long)status);
        exit(1);
    }
}

uint64_t monotonic_time_ns(void) {
    LARGE_INTEGER counter, freq;

    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);

    return (uint64_t)(
        (counter.QuadPart * 1000000000ULL) / freq.QuadPart
    );
}

#elifdef __linux__

#include <sys/random.h>
#include <time.h>

void generate_randoms(const size_t len) {
    ssize_t bytes_written = 0;
    while (bytes_written < len) {
        ssize_t ret_code = getrandom(
            &TEMP[bytes_written],
            len - bytes_written,
            GRND_RANDOM
        );

        if (ret_code >= 0)
            bytes_written += ret_code;
    }
}

#else
#error "Unsupported target platform!"
#endif

//Resets done to 0
void reset_done(cl_command_queue queue, uint32_t* done) {
    clEnqueueSVMMap(
        queue,
        CL_TRUE,
        CL_MAP_WRITE,
        done,
        sizeof(uint32_t),
        0,
        nullptr,
        nullptr
    );

    *done = 0;

    clEnqueueSVMUnmap(queue, done, 0, nullptr, nullptr);
}

//Gives `true` if done = 0, basically the simulation ends
bool is_done(cl_command_queue queue, uint32_t* done) {
    bool result = false;

    clEnqueueSVMMap(
        queue,
        CL_TRUE,
        CL_MAP_READ,
        done,
        sizeof(uint32_t),
        0,
        nullptr,
        nullptr
    );

    if (*done == 0) {
        result = true;
    }
    
    clEnqueueSVMUnmap(queue, done, 0, nullptr, nullptr);
    return result;
}

//Gets the current `logical` tree grid
uint8_t* get_tree_grid(uint8_t* a, uint8_t* b) {
    if (TFF)
        return b;
    else
        return a;
}

//Gets the next `logical` tree grid
uint8_t* get_next_tree_grid(uint8_t* a, uint8_t* b) {
    if (TFF)
        return a;
    else
        return b;
}

//Gets random bytes on linux such that, p(1) = 0.25 & p(0) = 0.75
void get_randoms(
    cl_command_queue queue,
    uint8_t *buffer,
    const size_t len
) {
    generate_randoms(len);

    for (size_t i = 0; i < len; i++)
        TEMP[i] = (TEMP[i] < 64) ? 1 : 0;

    clEnqueueSVMMemcpy(
        queue,
        CL_FALSE,
        buffer,
        TEMP,
        len,
        0,
        nullptr,
        nullptr
    );
}

//Initialises the tree grid to be simulated
void initialize_trees(
    cl_command_queue queue,
    uint8_t *buffer,
    const size_t len,
    const uint32_t tree_grid_size,
    const uint8_t factor,
    const bool print_enabled
) {
    generate_randoms(len);

    for (size_t i = 0; i < len; i++)
        TEMP[i] = (TEMP[i] <= factor && TEMP[i] >= 0) ? 1 : 0;

    for (size_t i = 0; i < tree_grid_size + 2; i++) {
        TEMP[i] = 3;
        TEMP[(tree_grid_size + 2) * (tree_grid_size + 1) + i] = 3;
    }

    for (size_t i = 1; i <= tree_grid_size; i++) {
        TEMP[i * (tree_grid_size + 2)] = 3;
        TEMP[i * (tree_grid_size + 2) + tree_grid_size + 1] = 3;
    }

    if (print_enabled) {
        printf("At start:\n");
        for (size_t j = 0; j < tree_grid_size; j++) {
            for (size_t i = 0; i < tree_grid_size; i++) {
                printf("%d", TEMP[(j + 1) * (tree_grid_size + 2) + i + 1]);
            }
            printf("\n");
        }
        printf("\n");
    }

    for (size_t j = 0; j < tree_grid_size; j++) {
        for (size_t i = 0; i < tree_grid_size; i++) {
            if (TEMP[(j + 1) * (tree_grid_size + 2) + i + 1] == 1)
                STATS[0] += 1;
            else if (TEMP[(j + 1) * (tree_grid_size + 2) + i + 1] == 0)
                STATS[1] += 1;
        }
    }

    clEnqueueSVMMemcpy(
        queue,
        CL_TRUE,
        buffer,
        TEMP,
        len,
        0,
        nullptr,
        nullptr
    );
}

//Let the user choose which device runs the simulation
cl_device_id choose_device() {
    uint32_t platform_count = 0;
    
    //Get all available OpenCL platforms
    clGetPlatformIDs(0, nullptr, &platform_count);
    cl_platform_id platforms[platform_count];
    if (platform_count == 0) {
        printf("No OpenCL platforms found! Simulation aborted!\n");
        exit(0);
    }
    clGetPlatformIDs(platform_count, platforms, nullptr);

    uint32_t total_device_count = 0;
    //Get total device count on the host system
    for(uint32_t i = 0; i < platform_count; i++) {
        uint32_t device_count = 0;
        clGetDeviceIDs(
            platforms[i],
            CL_DEVICE_TYPE_ALL,
            0,
            nullptr,
            &device_count
        );
        total_device_count += device_count;
    }
    if (total_device_count == 0) {
        printf("No OpenCL devices found! Simulation aborted!\n");
        exit(0);
    }
    
    cl_device_id all_devices[total_device_count];
    uint32_t idx = 0;
    //Get all devices
    for (uint32_t i = 0; i < platform_count; i++) {
        uint32_t device_count = 0;
        clGetDeviceIDs(
            platforms[i],
            CL_DEVICE_TYPE_ALL,
            0,
            nullptr,
            &device_count
        );
        clGetDeviceIDs(
            platforms[i],
            CL_DEVICE_TYPE_ALL,
            device_count,
            &all_devices[idx],
            nullptr
        );

        idx += device_count;
    }

    uint32_t supported_device_count = 0;
    cl_device_id supported_devices[total_device_count];
    //Filter out devices that dont support OpenCLv2+
    for (uint32_t i = 0; i < total_device_count; i++) {
        size_t version_data_len;
        clGetDeviceInfo(
            all_devices[i],
            CL_DEVICE_VERSION,
            0,
            nullptr,
            &version_data_len
        );
        char version_data[version_data_len];
        clGetDeviceInfo(
            all_devices[i],
            CL_DEVICE_VERSION,
            version_data_len,
            version_data,
            nullptr
        );

        if ((uint8_t) (version_data[7] - '0') >= 2) {
            supported_devices[supported_device_count] = all_devices[i];
            supported_device_count++;
        }
    }
    
    if (supported_device_count == 0) {
        printf("No supported OpenCL devices found! Simulation aborted!\n");
        exit(0);
    }
    else if (supported_device_count == 1) {
        return supported_devices[0];
    }
    else {
        printf("Supported devices available:\n");
        for(uint32_t i = 0; i < supported_device_count; i++) {
            size_t name_len;
            clGetDeviceInfo(
                supported_devices[i],
                CL_DEVICE_NAME,
                0,
                nullptr,
                &name_len
            );
            char name[name_len];
            clGetDeviceInfo(
                supported_devices[i],
                CL_DEVICE_NAME,
                name_len,
                name,
                nullptr
            );
        
            size_t vendor_name_len;
            clGetDeviceInfo(
                supported_devices[i],
                CL_DEVICE_VENDOR,
                0,
                nullptr,
                &vendor_name_len
            );
            char vendor_name[vendor_name_len];
            clGetDeviceInfo(
                supported_devices[i],
                CL_DEVICE_VENDOR,
                vendor_name_len,
                vendor_name,
                nullptr
            );
            printf("%u] [%s] %s\n", i + 1, vendor_name, name);
        }
        printf("\n");
    
        uint32_t idx2 = 0;
        printf("Choose device (number): ");
        scanf("%u", &idx2);
        if (idx2 < 1) {
            printf("\nInvalid selection! Input should be 1 or greater than that!\n");
            exit(1);
        }
        else if (idx2 > supported_device_count) {
            printf("\nInvalid selection! Device number doesnt exist!\n");
            exit(1);
        }
        else
            return supported_devices[idx2 - 1];
    }
}