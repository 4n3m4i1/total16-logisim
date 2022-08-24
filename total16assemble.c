// total16 assembler

// cmd: total16assemble (file in) (file out) (flags)

#include <stdio.h>
#include <string.h>             // strncpy pretty much
#include <inttypes.h>

#define INPUT_LINE_LIM  64
#define OUTPUT_BUFFER   16
#define LABEL_MAX_LEN   12

// CMD Arguments:
//  /file/path/first /outputname (flags)

void parse_label(char *input, char *output, int label_len_limit);   // Identify Labels, fill Label Table
int remove_leading_whitespace(char *arrayS, int arr_len);   // Removes leading whitespace and skips blank lines
int parser(char *in_str, char *out_str);                    // Parses OPCODE and ARGUMENTS
uint32_t hasher(char *tohash);                              // Converts char strings to unique numbers
void savNumBin(int opcode, char *outstr__);                 // Converts num to 6 bit 0/1 ascii string
int cvtRegister(char *arg);                                 // Converts register label to address

int savArg(char *arg_0_i, char *arg_1_i, char *output_arr); // Builds Instruction for register arithmetic
int savImm(char *arg_0_i, char *imm_i, char *output_arr);   // Builds Instruction for immediate arithmetic
int sav2Imm(char *imm_0, char *imm_1, char *output_arr);    // Builds Instruction for 2 immediate inputs
int savLon(char *lon_imm, char *output_arr);                // Save Long Immediate (10-bit)
int savJmp(uint32_t hash_input, char *output_arr);          // Links jump label to label table, builds jump instruction

int loiStr_toNum(char *in_);                                // 0-1023 char string -> int

int str2int(char *imm_arr);                                 // Convert immediate string to int
int loiStr_toNum(char *in_);

void fillOutput(char *output_ar, char *arg_0, char *arg_1); // Writes arguments to output buffer

void instructionError(char *type, int lineNum, int curr_pc); // Generic error messages w line no and pc

void instr2hex(char *instructionIn, char *tmp);             // Convert 16 bit instruction string to hex

uint8_t debug_bitfield = 0x00;

unsigned int LINE_NUM = 1;

uint16_t PC = 0x0000;
uint16_t LABEL_TABLE[32] = {0x0000};
uint32_t LINK_TABLE[32] = {0x00000000};
int TABLE_PTR = 0;

int main(int argc, char *argv[]){
    // Flags
    //  -d  ==  minor debug
    //  -D  ==  Major Debug





    char file_path_str[64] = {0x00};
    char file_output_name[64] = {0x00};

    char file_aux_name[63] = {0x00};            // name of hex output file

    char flag_read_in[4] = {0x00};

    if(argc < 3){
        printf("Please Provide proper input path and output name!\n");
        return 0;
    } else if(argc > 8){
        printf("Too many arguments provided!\n");
        return 0;
    }


    strncpy(file_path_str, argv[1], 63);
    file_path_str[63] = 0x00;

    strncpy(file_output_name, argv[2], 63);
    file_output_name[63] = 0x00;

    int pt = 0;
    for(pt; pt < 63; pt++){
        file_aux_name[pt] = file_output_name[pt];
        if(file_output_name[pt] == '.'){
            break;
        }
    }
    file_aux_name[pt + 1] = 'h';
    file_aux_name[pt + 2] = 'e';
    file_aux_name[pt + 3] = 'x';
    for(pt += 4; pt < 64; pt++){
        file_aux_name[pt] = 0x00;
    }

    for(int n = 3; n < argc; n++){                          // Flag Read In
        strncpy(flag_read_in, argv[n], 3);

        if(flag_read_in[0] == '-'){
            switch(flag_read_in[1]){
                case 'd':
                    printf("Debug Mode:\tLight\n");
                    debug_bitfield = 0x01;
                    break;
                case 'D':
                    printf("Debug Mode:\tMost\n");
                    debug_bitfield = 0x02;
                    break;
                case 'a':
                    printf("Debug Mode:\tAll\n");
                    debug_bitfield = 0x03;
                    break;
                case 'A':
                    printf("Debug Mode:\tAll + Line Numbers\n");
                    debug_bitfield = 0x04;
                    break;
                case 'H':
                    printf("Debug Move:\tHEX + All\n");
                    debug_bitfield = 0xFF;
                    break;
                default:
                    break;
            }
        }
    }

    if(debug_bitfield > 0){
        printf("Input Path:\t%s\n", file_path_str);
        printf("Output Name:\t%s\n\n", file_output_name);
    }


    // Open and check validity of files
    FILE *input_f;
    FILE *output_f;                             // Binary output

    FILE *outputHex;

    input_f = fopen(file_path_str, "r");        // Open input file for reading

    if(!input_f){
        // File isn't available for reading
        printf("File Not Accessible!!\n");
        return 1;
    }

    // At this rate we should at least have a real file to point to
    output_f = fopen(file_output_name, "w+");

    //
    outputHex = fopen(file_aux_name, "w+");

    char inbuff[INPUT_LINE_LIM];
    char outbuff[OUTPUT_BUFFER + 2] = {0x00};
    char LABEL_NAME[LABEL_MAX_LEN + 1];

    while(fgets(inbuff, INPUT_LINE_LIM, input_f)){          // Read line by line to build label table
        if(remove_leading_whitespace(inbuff, INPUT_LINE_LIM)){
            if(inbuff[0] == '.'){               // Label Detected

                parse_label(inbuff, LABEL_NAME, LABEL_MAX_LEN);

                if(debug_bitfield > 1){
                    printf("Label Detected:\t%s\n", LABEL_NAME);
                    printf("Label Address:\t%u\n", PC);
                    printf("Label Number:\t%u\n", TABLE_PTR);
                }

                LABEL_TABLE[TABLE_PTR] = PC;
                LINK_TABLE[TABLE_PTR] = hasher(LABEL_NAME);
                TABLE_PTR += 1;

                if(debug_bitfield > 1){
                    printf("\n");
                }
            } else {                            // Normal Instruction Detected
                PC += 1;                        // Just index PC
            }

            for(int n = 0; n < INPUT_LINE_LIM; n++){
                inbuff[n] = 0x00;
            }
            for(int n = 0; n < LABEL_MAX_LEN + 1; n++){
                LABEL_NAME[n] = 0x00;
            }
        } else {
            // Read line is just whitespace, read next line
        }
    }

    if(debug_bitfield){                                     // Print label table
        if(debug_bitfield > 1){
            printf("Label Table:\n");

            for(int n = 0; n < 32; n++){
                printf("\t%u\t\t%u\n", LINK_TABLE[n], LABEL_TABLE[n]);
            }
        }

        printf("\n");
        for(int n = 0; n < 96; n++){
            printf("~");
        }
        printf("\n\t\t\tEnd of Label Search, Begin Instruction Build\n");
        for(int n = 0; n < 96; n++){
            printf("~");
        }
        printf("\n");
    }

    fclose(input_f);                                        // Reset Read Pointer

    input_f = fopen(file_path_str, "r");                    // Open input file for reading

    if(!input_f){
        // File isn't available for reading
        printf("File Not Accessible!!\n");
        return 1;
    }

    PC = 0;

    while(fgets(inbuff, INPUT_LINE_LIM, input_f)){          // Read line by line to process instructions
        char toh[5] = {0x00};       // temp hex arr

        if(debug_bitfield > 3){
            printf("Line: %u\tPC: %u\n", LINE_NUM, PC);
        }
        if(remove_leading_whitespace(inbuff, INPUT_LINE_LIM)){
            if(inbuff[0] == '.'){               // Label Detected
                // Label detection, just skip
            } else {                            // Normal Instruction Detected
                if(parser(inbuff, outbuff)){
                    printf("Error on instruction parse!!\n");
                    return 1;
                } else {
                    instr2hex(outbuff, toh);
                    toh[4] = 0x00;

                    if(debug_bitfield){
                        printf("\n");

                        if(debug_bitfield > 1){
                            printf("Parsed Instruction:\t0x%s\t%s\n\n", toh, outbuff);
                        }
                    }

                    outbuff[16] = '\n';
                    fputs(outbuff, output_f);
                    //instr2hex(outbuff, toh);                // TEMPORARy
                    fputs(toh, outputHex);

                    if(PC % 2 == 1){
                        fputc('\n', outputHex);
                    } else {
                        fputc(' ', outputHex);
                    }

                    PC += 1;
                }
            }

            for(int n = 0; n < INPUT_LINE_LIM; n++){
                inbuff[n] = 0x00;
            }

            for(int n = 0; n < OUTPUT_BUFFER + 2; n++){
                outbuff[n] = 0x00;
            }

        } else {
            // Read line is just whitespace, read next line
        }
        LINE_NUM += 1;
    }

    fclose(input_f);
    fclose(output_f);
    fclose(outputHex);
    printf("\nOperation Complete!\n");
    return 0;
}

void parse_label(char *input, char *output, int label_len_limit){
    for(int n = 0; n < label_len_limit + 1; n++){
        if(input[n + 1] == ':'){
            break;
        }
        output[n] = input[n + 1];
    }

}

int remove_leading_whitespace(char *arrayS, int arr_len){                   // Remove leading \t and spaces
    char t_arr[arr_len];

    for(int n = 0; n < arr_len; n++){
        t_arr[n] = 0x00;
    }

    int n = 0;
    int ctr = 0;

    for(n; n < 4; n++){
        if(arrayS[n] == '\n'){
            return 0;
        }
    }

    for(n = 0; n < arr_len; n++){
        if(arrayS[n] != '\n' && arrayS[n] != '\r'){         // If char isn't newline
            if(arrayS[n + 1] == '\t'){                      // Replace tabs with spaces
                arrayS[n + 1] = ' ';
            }
            if(arrayS[n] > 0x19 && arrayS[n] < 0x7F){
                t_arr[ctr] = arrayS[n];
                ctr += 1;
            }
        } else {
            break;
        }
    }

    for(n = 0; n < arr_len - 1; n++){
        arrayS[n] = '\0';
    }

    for(n = 0; n < ctr + 1; n++){
        arrayS[n] = t_arr[n];
    }

    return 1;
}

int parser(char *in_str, char *out_str){                    // Parse string into 3 components, write to file if valid, else throw error
        int op_ptr = 0;
        char opcode_parse[8] = {0x00};
        char arg_parse[24] = {0x00};
        char arg_0[6] = {0x00};
        char arg_1[6] = {0x00};

        char zzzz[6] = {'0','\0','\0','\0','\0','\0'};

        /////////////////////////////////////////////////
        if(debug_bitfield > 2){
            printf("OPCODE:\t");
        }

        int n = 0, ctr = 0;
        for(n = 0; n < INPUT_LINE_LIM; n++){                // Find OPCODE
            if(in_str[n] == ' ' || in_str[n] == '\t' || in_str[n] == '\n' || in_str[n] == '\0'){
                break;
            } else {
                opcode_parse[n] = in_str[n];

                if(debug_bitfield > 2){
                    printf("%i ", (int)opcode_parse[n]);
                }
            }
        }

        if(debug_bitfield > 2){
            printf("\nARG 0:\t");
        }
        /////////////////////////////////////////////////
        ctr = 0;
        for(n = n + 1; n < INPUT_LINE_LIM; n++){            // Find ARG0
            if(in_str[n] == ' ' || in_str[n] == '\t' || in_str[n] == '\n' || in_str[n] == '\0'){
                break;
            } else {
                arg_0[ctr] = in_str[n];

                if(debug_bitfield > 2){
                    printf("%i ", (unsigned int)arg_0[ctr]);
                }

                ctr += 1;
            }
        }

        if(debug_bitfield > 2){
            printf("\nARG 1:\t");
        }
        /////////////////////////////////////////////////
        ctr = 0;
        for(n = n + 1; n < INPUT_LINE_LIM; n++){            // Fine ARG1
            if(in_str[n] == ' ' || in_str[n] == '\t' || in_str[n] == '\n' || in_str[n] == '\0'){
                break;
            } else {
                arg_1[ctr] = in_str[n];

                if(debug_bitfield > 2){
                    printf("%i ", (unsigned int)arg_1[ctr]);
                }

                ctr += 1;
            }
        }

        if(debug_bitfield > 2){
            printf("\n");
        }
        /////////////////////////////////////////////////
        if(debug_bitfield > 0){
            printf("INPUT:\t%s\n", in_str);
        }

        if(debug_bitfield > 2){
            printf("OPCODE:\t%s\n", opcode_parse);
            printf("Arg 0: %s\tArg 1: %s\n", arg_0, arg_1);
        }
        //=========================================== OPCODE CASES ==============================//
        switch(hasher(opcode_parse)){
            case 8868:
            case 12420:
                // NOP or nop
                if(debug_bitfield > 1){
                    printf("Match Found:\tNOP\n");
                }
                savNumBin(0x00, out_str);
                savNumBin(0x00, arg_0);
                savNumBin(0x00, arg_1);
                fillOutput(out_str, arg_0, arg_1);
            break;

            case 9467:
            case 13019:
                // MOV or mov
                if(debug_bitfield > 1){
                    printf("Match Found:\tMOV\n");
                }
                savNumBin(0x01, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8953:
            case 12505:
                // SWP or swp
                if(debug_bitfield > 1){
                    printf("Match Found:\tSWP\n");
                }
                savNumBin(0x02, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 806:
            case 1158:
                // LI or li
                if(debug_bitfield > 1){
                    printf("Match Found:\tLI\n");
                }
                savNumBin(0x03, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 7545:
            case 11097:
                // ADD or add
                if(debug_bitfield > 1){
                    printf("Match Found:\tADD\n");
                }
                savNumBin(0x04, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 75535:
            case 111087:
                // UADD or uadd
                if(debug_bitfield > 1){
                    printf("Match Found:\tUADD\n");
                }
                savNumBin(0x05, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 7533:
            case 11085:
                // SUB or sub
                if(debug_bitfield > 1){
                    printf("Match Found:\tSUB\n");
                }
                savNumBin(0x06, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 75415:
            case 110967:
                // USUB or usub
                if(debug_bitfield > 1){
                    printf("Match Found:\tUSUB\n");
                }
                savNumBin(0x07, out_str);
            break;

            case 80545:
            case 116097:
                // ADDI or addi
                if(debug_bitfield > 1){
                    printf("Match Found:\tADDI\n");
                }
                savNumBin(0x08, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 80533:
            case 116085:
                // SUBI or subi
                if(debug_bitfield > 1){
                    printf("Match Found:\tSUBI\n");
                }
                savNumBin(0x09, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 8527:
            case 12079:
                // MUL or mul
                if(debug_bitfield > 1){
                    printf("Match Found:\tMUL\n");
                }
                savNumBin(0x0A, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 85346:
            case 120898:
                // LMUL or lmul
                if(debug_bitfield > 1){
                    printf("Match Found:\tLMUL\n");
                }
                savNumBin(0x0B, out_str);
            break;

            case 9398:
            case 12950:
                // DIV or div
                if(debug_bitfield > 1){
                    printf("Match Found:\tDIV\n");
                }
                savNumBin(0x0C, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8166:
            case 11718:
                // LOI or loi
                if(debug_bitfield > 1){
                    printf("Match Found:\tLOI\n");
                }
                savNumBin(0x0D, out_str);
                if(savLon(arg_0, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8506:
            case 12058:
                // LSL or lsl
                if(debug_bitfield > 1){
                    printf("Match Found:\tLSL\n");
                }
                savNumBin(0x0E, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8206:
            case 11758:
                // LSI or lsi
                if(debug_bitfield > 1){
                    printf("Match Found:\tLSI\n");
                }
                savNumBin(0x0F, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 8512:
            case 12064:
                // RSL or rsl
                if(debug_bitfield > 1){
                    printf("Match Found:\tRSL\n");
                }
                savNumBin(0x10, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8212:
            case 11764:
                // RSI or rsi
                if(debug_bitfield > 1){
                    printf("Match Found:\tRSI\n");
                }
                savNumBin(0x11, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 7412:
            case 10964:
                // RSA or rsa
                if(debug_bitfield > 1){
                    printf("Match Found:\tRSA\n");
                }
                savNumBin(0x12, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 85272:
            case 120824:
                // ROTL or rotl
                if(debug_bitfield > 1){
                    printf("Match Found:\tROTL\n");
                }
                savNumBin(0x13, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 7645:
            case 11197:
                // AND or and
                if(debug_bitfield > 1){
                    printf("Match Found:\tAND\n");
                }
                savNumBin(0x14, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 899:
            case 1251:
                // OR or or
                if(debug_bitfield > 1){
                    printf("Match Found:\tOR\n");
                }
                savNumBin(0x15, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 9078:
            case 12630:
                // XOR or xor
                if(debug_bitfield > 1){
                    printf("Match Found:\tXOR\n");
                }
                savNumBin(0x16, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 9268:
            case 12820:
                // NOT or not
                if(debug_bitfield > 1){
                    printf("Match Found:\tNOT\n");
                }
                savNumBin(0x17, out_str);
                if(savImm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 80645:
            case 116197:
                // ANDI or andi
                if(debug_bitfield > 1){
                    printf("Match Found:\tANDI\n");
                }
                savNumBin(0x18, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 8199:
            case 11751:
                // ORI or ori
                if(debug_bitfield > 1){
                    printf("Match Found:\tORI\n");
                }
                savNumBin(0x19, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 726:
            case 1078:
                // LA or la
                if(debug_bitfield > 1){
                    printf("Match Found:\tLA\n");
                }
                savNumBin(0x1A, out_str);
            break;

            case 946:
            case 1298:
                // LW or lw
                if(debug_bitfield > 1){
                    printf("Match Found:\tLW\n");
                }
                savNumBin(0x1B, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 953:
            case 1305:
                // SW or sw
                if(debug_bitfield > 1){
                    printf("Match Found:\tSW\n");
                }
                savNumBin(0x1C, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 8246:
            case 11798:
                // LWI or lwi
                if(debug_bitfield > 1){
                    printf("Match Found:\tLWI\n");
                }
                savNumBin(0x1D, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 8253:
            case 11805:
                // SWI or swi
                if(debug_bitfield > 1){
                    printf("Match Found:\tSWI\n");
                }
                savNumBin(0x1E, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 88737:
            case 124289:
                // CMEQ or cmeq
                if(debug_bitfield > 1){
                    printf("Match Found:\tCMEQ\n");
                }
                savNumBin(0x20, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 92437:
            case 127989:
                // CMLT or cmlt
                if(debug_bitfield > 1){
                    printf("Match Found:\tCMLT\n");
                }
                savNumBin(0x21, out_str);
                if(savArg(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            case 97737:
            case 133289:
                // CMEZ or cmez
                if(debug_bitfield > 1){
                    printf("Match Found:\tCMEZ\n");
                }
                savNumBin(0x22, out_str);
                if(savImm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 80737:
            case 116289:
                // CMEI or cmei
                if(debug_bitfield > 1){
                    printf("Match Found:\tCMEI\n");
                }
                savNumBin(0x23, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 81437:
            case 116989:
                // CMLI or cmli
                if(debug_bitfield > 1){
                    printf("Match Found:\tCMLI\n");
                }
                savNumBin(0x24, out_str);
                if(savImm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 9213:
            case 12765:
                // SIT or sit
                if(debug_bitfield > 1){
                    printf("Match Found:\tSIT\n");
                }
                savNumBin(0x28, out_str);
                if(sav2Imm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 7813:
            case 11365:
                // SIF or sif
                if(debug_bitfield > 1){
                    printf("Match Found:\tSIF\n");
                }
                savNumBin(0x29, out_str);
                if(sav2Imm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 74:
            case 106:
                // J or j
                if(debug_bitfield > 1){
                    printf("Match Found:\tJ\n");
                }
                savNumBin(0x2A, out_str);
                if(savJmp(hasher(arg_0), out_str)){
                    if(debug_bitfield){
                        printf("Error: No Label Found For %s!!\n", arg_0);
                    }
                    return 1;                           // Error, no label found
                } else {
                    if(debug_bitfield > 1){
                        printf("Success: Label Found For %s!\n", arg_0);
                    }
                }
            break;

            case 8324:
            case 11876:
                // JAL or jal
                if(debug_bitfield > 1){
                    printf("Match Found:\tJAL\n");
                }
                savNumBin(0x2B, out_str);
            break;

            case 81230:
            case 116782:
                // PUSH or push
                if(debug_bitfield > 1){
                    printf("Match Found:\tPUSH\n");
                }
                savNumBin(0x2E, out_str);
                if(savImm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 8870:
            case 12422:
                // POP or pop
                if(debug_bitfield > 1){
                    printf("Match Found:\tPOP\n");
                }
                savNumBin(0x2F, out_str);
                if(savImm(arg_0, zzzz, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We good
                }
            break;

            case 84326273:
            case 119881825:
                // SYSCALL or syscall
                if(debug_bitfield > 1){
                    printf("Match Found:\tSYSCALL\n");
                }
                savNumBin(0x3F, out_str);
                if(sav2Imm(arg_0, arg_1, out_str)){
                    instructionError(in_str, LINE_NUM, PC);
                    return 1;
                } else {
                    // We all good B^)
                }
            break;

            default:
                printf("Error on Parse: Unrecognized text in operator field.\n");
                instructionError(in_str, LINE_NUM, PC);
                return 1;
        }

        return 0;
}

uint32_t hasher(char *tohash){                      // Returns unique 32 bit hash per input string
    uint32_t t_sum = 0;
    uint32_t scalar = 1;

    if(debug_bitfield > 2){
        printf("HASH_INPUT:\t");
    }

    for(int n = 0; n < 24; n++){
        if(debug_bitfield > 2){
            printf("%i ", (int)tohash[n]);
        }
        if((tohash[n] < 0x7b && tohash[n] > 0x40) || (tohash[n] < 0x3A && tohash[n] > 0x2F)){   // Generate hash from valid chars
            t_sum += scalar * (uint32_t)tohash[n];
            scalar *= 10;
        }
        if(!tohash[n + 1]){                         // Break loop if next char is terminator
            break;
        }
    }

    if(debug_bitfield > 2){
        printf("\n");
        printf("Hash: %s == %u\n", tohash, t_sum);
    }

    return t_sum;
}

void savNumBin(int opcode, char *outstr__){         // Convert opcode value to 6 bit ASCII binary
    int t__ = 0x20;                             // 1 << 5
    for(int n = 0; n < 6; n++){
        if(opcode & t__){
            outstr__[n] = '1';
        } else {
            outstr__[n] = '0';
        }
        t__ = t__ >> 1;
    }
}

int cvtRegister(char *arg){                         // Convert register alias to register address
    switch(hasher(arg)){
        case 48:
            // $0 register
            return 0;
            break;

        case 57071:
        case 60623:
            // GPR0, gpr0, $GPR0, $gpr0
            return 1 << 1;                      // Shift output as our parser expects 6 bit output
            break;                              // for opcode instead of 5 bit for register address

        case 58071:
        case 61623:
            // GPR1, gpr1, $GPR1, $gpr1
            return 2 << 1;
            break;

        case 59071:
        case 62623:
            // GPR2, gpr2, $GPR2, $gpr2
            return 3 << 1;
            break;

        case 60071:
        case 63623:
            // GPR3, gpr3, $GPR3, $gpr3
            return 4 << 1;
            break;

        case 61071:
        case 64623:
            // GPR4, gpr4, $GPR4, $gpr4
            return 5 << 1;
            break;

        case 62071:
        case 65623:
            // GPR5, gpr5, $GPR5, $gpr5
            return 6 << 1;
            break;

        case 63071:
        case 66623:
            // GPR6, gpr6, $GPR6, $gpr6
            return 7 << 1;
            break;

        case 64071:
        case 67623:
            // GPR7, gpr7, $GPR7, $gpr7
            return 8 << 1;
            break;

        default:
            return -1;
    }
}

int savArg(char *arg_0_i, char *arg_1_i, char *output_arr){ // Convert 2 register operations, fill output
    int reg_0 = cvtRegister(arg_0_i);
    int reg_1 = cvtRegister(arg_1_i);

    char t_0[6] = {0x00};
    char t_1[6] = {0x00};

    if(reg_0 < 0 || reg_1 < 0){
        // Error on input!
        printf("Error: Invalid Register Input!!\n");
        return 1;
    } else {
        savNumBin(reg_0, t_0);
        savNumBin(reg_1, t_1);
        fillOutput(output_arr, t_0, t_1);
        return 0;
    }
}

int savImm(char *arg_0_i, char *imm_i, char *output_arr){
    int reg_0 = cvtRegister(arg_0_i);
    int reg_1 = str2int(imm_i);

    char t_0[6] = {0x00};
    char t_1[6] = {0x00};

    if(reg_0 < 0){
        printf("Error: Invalid Register Input!!\n");
        return 1;
    } else if (reg_1 < 0 || reg_1 > 62){
        printf("Error: Out of Bounds Immediate!!\n");
        return 1;
    } else {
        savNumBin(reg_0, t_0);
        savNumBin(reg_1, t_1);
        fillOutput(output_arr, t_0, t_1);
        return 0;
    }
}

int sav2Imm(char *imm_0, char *imm_1, char *output_arr){
    int reg_0 = str2int(imm_0);
    int reg_1 = str2int(imm_1);

    //printf("Sav2Imm\n");
    //printf("R0: %s\tR1: %s\n", imm_0, imm_1);
    //printf("I0: %i\tI1: %i\n", reg_0, reg_1);

    char t_0_[6] = {0x00};
    char t_1_[6] = {0x00};

    if (reg_1 < 0 || reg_1 > 62 || reg_0 < 0 || reg_0 > 62){
        printf("Error: Out of Bounds Immediate!!\n");
        return 1;
    } else {
        savNumBin(reg_0, t_0_);
        savNumBin(reg_1, t_1_);
        fillOutput(output_arr, t_0_, t_1_);
        return 0;
    }
}

int str2int(char *imm_arr){                                 // Up to 2 char to int 5 bit output
    int ptr = 0;
    int valid_ct = 0;
    for(ptr; ptr < 3; ptr++){
        if(imm_arr[ptr] && (imm_arr[ptr] > 0x2F && imm_arr[ptr] < 0x3A)){
            // We have a 0-9 ASCII
            //printf("%i ", (int)imm_arr[ptr]);
            valid_ct += 1;
        }
        else if(!imm_arr[ptr]){
            // Null Terminator
            break;
        } else {
            // Invalid Character
            return -999;
        }
    }

    int scale;
    if(valid_ct > 1){
        scale = 10;
    } else {
        scale = 1;
    }

    int temp = 0;

    for(ptr = 0; ptr < valid_ct; ptr++){
        temp += ((int)imm_arr[ptr] - 0x30) * scale;
        scale /= 10;
    }

    if(debug_bitfield > 2){
        printf("Temp: %i\n", temp);
    }

    return temp << 1;
}

int savLon(char *lon_imm, char *output_arr){
    int reg_0 = loiStr_toNum(lon_imm);

    //printf("Val: %u\n", reg_0);

    int reg_1 = (reg_0) & 0b11111;
    reg_0 = (reg_0 >> 5) & 0b11111;

    reg_0 = reg_0 << 1;
    reg_1 = reg_1 << 1;

    //printf("R0: %i\tR1: %i\n", reg_0, reg_1);

    char t_0[6] = {0x00};
    char t_1[6] = {0x00};

    if(reg_0 < 0 || reg_0 > 1023){
        printf("Error: Long Immediate Out of Bounds!\n");
        return 1;
    } else {
        savNumBin(reg_0, t_0);
        savNumBin(reg_1, t_1);
        fillOutput(output_arr, t_0, t_1);
        return 0;
    }
}

int loiStr_toNum(char *in_){
    int str_len_ = 0;

    for(str_len_; str_len_ < 6; str_len_++){
        if(!in_[str_len_]){
            break;
        }
    }

    if(!str_len_){
        printf("Error: No Argument Provided for LOI Load\n");
        return -1;
    }

    int scalar = 1;

    for(int n = 1; n < str_len_; n++){
        scalar *= 10;
    }

    //printf("Scalar: %i\tIn: %s\n", scalar, in_);

    int tmp = 0;
    for(int n = 0; n < str_len_; n++){
        tmp += ((int)in_[n] - 0x30) * scalar;
        scalar /= 10;
    }

    //printf("Result: %i\n", tmp);
    return tmp;
}

int savJmp(uint32_t hash_input, char *output_arr){
    int n = 0;
    char t_arr[6] = {0x00};
    char z_arr[6] = {'0', '0', '0', '0', '0', 0x00};

    for(n; n < 33; n++){
        if(LINK_TABLE[n] == hash_input){
            break;                          // n contains matching index
        }
    }
    if(n > 31){
        return 1;                           // Error, no matching label found
    } else {
        savNumBin(LABEL_TABLE[n], t_arr);
        fillOutput(output_arr, z_arr, t_arr);

        if(debug_bitfield){
            printf("Label Found:\t%i\n", LABEL_TABLE[n]);
            if(debug_bitfield > 2){
                printf("JMP ADDR:\t");
                for(int m = 0; m < 6; m++){
                    printf("%c", t_arr[m]);
                }
                printf("\n\n");
            }
        }

        //output arr == 16 bit
        // fill last
        return 0;
    }
}

void fillOutput(char *output_ar, char *arg_0, char *arg_1){ // Fill 10 bit argument component
    // Output Format:                                       //  of instruction, OPCODE filled
    // (opcode already filled)                              //  already
    //  OOOO OOAA AAAa aaaa
    //  0123 4567 89AB CDEF
    for(int n = 0; n < 5; n++){
        output_ar[0x6 + n] = arg_0[n];
        output_ar[0xB + n] = arg_1[n];
       // printf("%c and %c", arg_0[n], arg_1[n]);
    }

    //printf("\nOutput S: %s\n", output_ar);
}

void instructionError(char *type, int lineNum, int curr_pc){
    printf("Error while assembling:\n");
    printf("\tInstruction:\t%s\n", type);

    if(lineNum > -1){
        printf("\t\tLine:\t%u\n\t\tPC:\t%u\n", lineNum, curr_pc);
    }
}

void instr2hex(char *instructionIn, char *tmp){             // Convert 16 bit binary instruction into 4 char hex format
    //char tmp[5] = {0x00};

    uint8_t accum = 0x00;

    for(int n = 0; n < 4; n++){
        for(int m = 0; m < 4; m++){
            //t_out[m] = instructionIn[m + (n * 4)];
            if(instructionIn[m + (n * 4)] == '1'){
                accum |= (1 << (3 - m));
            }
        }   // eol

        if(debug_bitfield == 15){
            printf("Accu: %u\n", accum);
        }

        if(accum < 10){
            tmp[n] = accum + 0x30;
        } else {
            tmp[n] = accum + 0x37;
        }
        accum = 0x00;
    }
    if(debug_bitfield == 15){
        printf("Instruction in HEX: %s\n", tmp);
    }
}

