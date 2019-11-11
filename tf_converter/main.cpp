#include <bits/stdc++.h>
#include <cassert>

// lol
//
#define private public
#define final

#include "input/graph.h"

class Converter : public Graph
{
public:
    void Convert(std::string className)
    {
        const int kAlign = ::xla::cpu_function_runtime::kAlign;
        auto ShouldAllocateBuffer = [](const ::xla::cpu_function_runtime::BufferInfo& buffer) {
            return buffer.is_temp_buffer() || buffer.is_entry_parameter();
        };
        auto RoundUp = [kAlign](int value) {
            return (value + kAlign - 1) / kAlign * kAlign;
        };
        const ::xla::cpu_function_runtime::BufferInfo* bufferInfos = BufferInfos();
        int* offsets = new int[kNumBuffers];
        int* sizes = new int[kNumBuffers];
        int totalSize = 0;
        for (int i = 0; i < kNumBuffers; i++)
        {
            if (ShouldAllocateBuffer(bufferInfos[i]))
            {
                offsets[i] = totalSize;
                sizes[i] = bufferInfos[i].size();
                totalSize += RoundUp(sizes[i]);
            }
            else
            {
                offsets[i] = -1;
                sizes[i] = -1;
            }
        }

        std::string prefix = "____" + className + "_x_";

        printf("// Generated, DO NOT EDIT!\n//\n\n");
        printf("#pragma once\n#include \"indigo_utils.h\"\n\n");
        printf("void __xla___graph(\n"
               "\tvoid* result, const void* run_options,\n"
               "\tconst void** args, void** temps, void* profile_counters);\n\n");

        printf("// Buffer Layout Information\n//\n");
        printf("static const int %snumBuffers = %d;\n", prefix.c_str(), (int)kNumBuffers);
        printf("static const int %sbufferOffsets[%d] = {\n\t", prefix.c_str(), (int)kNumBuffers);
        for (int i = 0; i < (int)kNumBuffers; i++)
        {
            if (i > 0) { printf(", "); }
            printf("%d", offsets[i]);
        }
        printf("\n};\n");
        printf("static const int %sbufferSizes[%d] = {\n\t", prefix.c_str(), (int)kNumBuffers);
        for (int i = 0; i < (int)kNumBuffers; i++)
        {
            if (i > 0) { printf(", "); }
            printf("%d", sizes[i]);
        }
        printf("\n};\n");
        printf("static const int %sbufferLength = %d;\n", prefix.c_str(), totalSize);

        printf("\n// Parameters Layout Information\n//\n");
        printf("static const int %snumArgs = %d;\n", prefix.c_str(), (int)kNumArgs);
        printf("static const int %sargIndexes[%d] = {\n\t", prefix.c_str(), (int)kNumArgs);
        for (int i = 0; i < (int)kNumArgs; i++)
        {
            if (i > 0) { printf(", "); }
            printf("%d", ArgIndexToBufferIndex()[i]);
        }
        printf("\n};\n");
        int numResults = bufferInfos[kResultIndex].size() / 8;
        printf("static const int %snumResults = %d;\n", prefix.c_str(), numResults);
        printf("static const int %sresultIndexArrayIndex = %d;\n", prefix.c_str(), (int)kResultIndex);

        printf("\n// Default runtime options\n//\n");
        {
            xla::ExecutableRunOptions options;
            static_assert(sizeof(xla::ExecutableRunOptions) % 8 == 0, "tron");
            int len = sizeof(xla::ExecutableRunOptions) / 8;
            uint64_t* addr = reinterpret_cast<uint64_t*>(&options);
            printf("static const u64 %sruntimeOptions[%d] = {\n\t", prefix.c_str(), len);
            for (int i = 0; i < len; i++)
            {
                if (i > 0) { printf(", "); }
                printf("%llu", (unsigned long long)addr[i]);
            }
            printf("\n};\n");
        }

        printf("\nstruct %s {\n", className.c_str());
        printf("\tvoid** m_bufferList;\n");
        printf("};\n\n");

        printf("// Member functions to set parameters and retrieve results\n");
        printf("// TODO: We currently assume all parameter types are floats\n//\n");
        printf("float* %s_get_address_for_argument(struct %s* nn, int index) {\n", className.c_str(), className.c_str());
        printf("\tAssert(0 <= index && index < %snumArgs && nn->m_bufferList[%sargIndexes[index]] != NULL);\n", prefix.c_str(), prefix.c_str());
        printf("\treturn (float*)(nn->m_bufferList[%sargIndexes[index]]);\n}\n\n", prefix.c_str());

        printf("int %s_get_argument_size_bytes(int index) {\n", className.c_str());
        printf("\tAssert(0 <= index && index < %snumArgs && %sbufferSizes[%sargIndexes[index]] != -1);\n", prefix.c_str(), prefix.c_str(), prefix.c_str());
        printf("\treturn %sbufferSizes[%sargIndexes[index]];\n}\n\n", prefix.c_str(), prefix.c_str());

        printf("float* %s_get_address_for_result(struct %s* nn, int index) {\n", className.c_str(), className.c_str());
        printf("\tAssert(0 <= index && index < %snumResults);\n", prefix.c_str());
        printf("\treturn ((float**)(nn->m_bufferList[%sresultIndexArrayIndex]))[index];\n}\n\n", prefix.c_str());

        printf("// Member functions to construct, destruct and run the NN\n//\n");

        int roundUpHeaderSize = RoundUp(sizeof(void*) * kNumBuffers);
        printf("int WARN_UNUSED %s_constructor(struct %s* nn) {\n", className.c_str(), className.c_str());
        printf("\tint i;\n");
        printf("\tnn->m_bufferList = (void**)vmalloc(%sbufferLength + %d);\n", prefix.c_str(), roundUpHeaderSize);
        printf("\tif (nn->m_bufferList == NULL) { return 0; }\n");
        printf("\tAssert((((uintptr_t)(nn->m_bufferList)) & 4095) == 0);\n");
        printf("\tfor (i = 0; i < %snumBuffers; i++) {\n", prefix.c_str());
        printf("\t\tif(%sbufferOffsets[i] == -1) {\n\t\t\tnn->m_bufferList[i] = NULL;\n", prefix.c_str());
        printf("\t\t} else {\n\t\t\tnn->m_bufferList[i] = ((char*)nn->m_bufferList) + %d + %sbufferOffsets[i];\n\t\t}\n\t}\n", roundUpHeaderSize, prefix.c_str());
        printf("\treturn 1;\n}\n\n");

        printf("void %s_destructor(struct %s* nn) {\n", className.c_str(), className.c_str());
        printf("\tAssert(nn->m_bufferList != NULL);\n");
        printf("\tvfree(nn->m_bufferList);\n\tnn->m_bufferList = NULL;\n}\n\n");

        printf("void %s_run(struct %s* nn) {\n", className.c_str(), className.c_str());
        printf("\tAssert(nn->m_bufferList != NULL);\n");
        printf("\tfunction_call_5_params_respecting_stack_alignment(\n");
        printf("\t\t(void*)__xla___graph,\n");
        printf("\t\t(u64)(nn->m_bufferList[____indigo_nn_x_resultIndexArrayIndex]),\n");
        printf("\t\t(u64)(____indigo_nn_x_runtimeOptions),\n");
        printf("\t\t(u64)(NULL),\n");
        printf("\t\t(u64)(nn->m_bufferList),\n");
         printf("\t\t(u64)(NULL));\n");
        printf("}\n");
    }
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: ./main [output class name]");
        abort();
    }
    std::string className = argv[1];
    Converter g;
    g.Convert(className);
    return 0;
}

