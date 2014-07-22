# coding=utf-8 
# FOFOLITO - Sistema Operacional para RaspberryPi
#
# Programa que constroi nosso kernel
# Razões: eu não quero aprender a escrever makefiles e outros sistemas me 
#         pareceram bem confusos para essa tarefa.
# Basicamente, guardamos em um banco de dados a data da modificação dos fontes,
# sempre que o programa detectar que houve algum modificação, o projeto é
# recompilado e o banco de dados é salvo.
# Marcos Medeiros
__author__ = 'marcos'
import os
import sys
import shelve

from collections import OrderedDict

project_name = 'kernel'
project_img = project_name + '.img'

# Flag para compilar uma imagem que rode no QEMU
emulator_version = True
debug_version = True

# =============================================================================
# Definicoes padroes para a toolchain
# =============================================================================
defines = list()
if emulator_version:
    defines.append('__EMUVERSION__')
defines_list =  (' '.join([ '-D'+d for d in defines ]))
toolchain = '/opt/gcc-arm/bin/arm-none-eabi-'
c_compiler = toolchain + 'gcc'
cpu_model = '-mcpu=arm926ej-s'
c_flags = '-Iinclude -mcpu=arm1176jzf-s -marm -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wall -Wextra -nostartfiles -ffreestanding -nodefaultlibs -nostdinc -nostdlib -ffreestanding -fno-builtin -fomit-frame-pointer -std=gnu99 -c '  + defines_list
if debug_version:
    c_flags = c_flags + ' -g -O3'
else:
    c_flags = c_flags + ' -O3'
asm_compiler = toolchain + 'as'
asm_preprocessor_flags  = '-Iinclude -nostdinc -E -mcpu=arm1176jzf-s ' + defines_list
asm_flags = '-Iinclude -mcpu=arm1176jzf-s'
linker = toolchain + 'ld'
linker_script = str()
if emulator_version:
    linker_script = 'kernel-higher.ld'
else:
    linker_script = 'kernel.ld'
linker_flags = '-Map kernel.map --no-undefined -T ' + linker_script
objcopy = toolchain + 'objcopy'
objcopy_flags = '-O binary '
build_db = 'build.dat'
object_path = 'obj/'

# =============================================================================
# Regras
# =============================================================================
def compile_c_src(name):
    print('CC   ' + name)
    ret = os.system('%s %s %s -o %s' % (c_compiler, c_flags, name, src_files[name]) )
    os.system('%s %s %s -S -o %s' % (c_compiler, c_flags, name, src_files[name] + '.S') )
    return ret

def compile_asm_src(name):
    print('AS   ' + name)
    # cria o arquivo pré-processado
    ret = os.system('%s %s %s -o %s' % (c_compiler, asm_preprocessor_flags, name, name + '.0') )
    if ret != 0:
        return 1
    ret = os.system('%s %s %s -o %s' % (asm_compiler, asm_flags, name + '.0', src_files[name]) )
    # remove o arquivo pré-processado
    if os.path.exists(name + '.0'):
        os.remove(name + '.0')
    return ret
None 
# =============================================================================
# Arquivos do projeto
# =============================================================================
src_files = OrderedDict()
src_files = {
    'arch/init.S'           : object_path + 'init.o',
    'arch/start.S'          : object_path + 'start.o',
    'arch/entry.S'          : object_path + 'entry.o',
    'arch/mmu.S'			: object_path + 'mmu.o',
    'arch/irq.c'            : object_path + 'irq.o',
    'arch/exception.c'      : object_path + 'exception.o',
    'arch/page.c'           : object_path + 'page.o',
    'arch/ioremap.c'        : object_path + 'ioremap.o',
    'arch/setup.c'          : object_path + 'setup.o',
    'arch/switch.S'         : object_path + 'switch.o',
    'kernel/kmain.c'        : object_path + 'kmain.o',
    'kernel/fb.c'           : object_path + 'fb.o',
    'kernel/printk.c'       : object_path + 'printk.o',
    'kernel/sched.c'       : object_path + 'sched.o',
    'kernel/semaphore.c'       : object_path + 'semaphore.o',
    'mm/mm.c'               : object_path + 'mm.o',
    'mm/kmalloc.c'          : object_path + 'kmalloc.o',
    'mm/paging.c'           : object_path + 'paging.o',
    'lib/div.c'             : object_path + 'div.o',
    'lib/memory.c'          : object_path + 'memory.o',
    'lib/memory_s.S'        : object_path + 'memory_s.o',
    'driver/bcm2835_mbox.c' : object_path + 'bcm2835_mbox.o',
    'driver/bcm2835_fb.c'   : object_path + 'bcm2835_fb.o',
    'driver/bcm2835_gpio.c' : object_path + 'bcm2835_gpio.o',
    'driver/bcm2835_timer.c': object_path + 'bcm2835_timer.o',
    'driver/bcm2835_emmc.c' : object_path + 'bcm2835_emmc.o',
    'driver/ramdisk.c'      : object_path + 'ramdisk.o',
    'driver/fb_console.c'   : object_path + 'fb_console.o',
    'driver/fb_font_vga8x16.c'      : object_path + 'fb_font_vga8x16.o',
    
}

# Guarda a ultima modificação de um arquivo
src_files_modify = dict()
# Guarda a informação se é necessário ou não recompilar um arquivo
src_files_recompile = dict()

# =============================================================================
# Retorna uma string contendo todos os objetos
# =============================================================================
def get_objects():
    obj = ' '.join(src_files.values())
    return obj


# =============================================================================
# Verifica quais arquivos necessitam ser recompilados
# =============================================================================
def check_for_modified():
    try:
        db = shelve.open(build_db, flag='r')
        print('Verificando no banco de dados arquivos que necessitam ser recompilados...')
        for f in db:
            try:
                if db[f] != src_files_modify[f] or not os.path.exists(src_files[f]):
                    #print(f + ' alterado, necessário recompilar')
                    src_files_recompile[f] = True
                else:
                    src_files_recompile[f] = False
            except KeyError:
                src_files_recompile[f] = True
        db.close()
    except:
        pass

# =============================================================================
# Compila os arquivos do projeto
# =============================================================================
def do_compile():
    for f in src_files:
        if src_files_recompile[f]:
            ext = os.path.splitext(f)
            if ext[1] == '.S':
                if compile_asm_src(f):
                   return 1
            elif ext[1] == '.c':
                if compile_c_src(f):
                    return 1
            # opa, conseguimos compilar
            src_files_recompile[f] = False
    return 0

def do_link():
    objs = get_objects()
    print('LD   ' + project_img)
    ret = os.system('%s %s %s -o %s' % (linker, objs, linker_flags, project_name + '.elf') )
    if ret == 0:
        ret = os.system('%s %s %s %s' % (objcopy, project_name + '.elf', objcopy_flags, project_img) )
#        os.remove(project_name + '.elf')
        return ret
    return ret

# =============================================================================
# Salva o banco de dados
# =============================================================================
def save_database():
    try:
        db = shelve.open(build_db, flag='c')
        for f in src_files_modify:
            db[f] = src_files_modify[f]
        db.close()
    except:
        print('Não foi possível criar o banco de dados')

# =============================================================================
# Função principal
# =============================================================================
if __name__ == '__main__':
    if len(sys.argv) < 2:
        # Primeiro vamos verificar se os arquivos existem
        for f in src_files.keys():
            if not os.path.exists(f):
                del src_files[f]
            else:
                # se existir, então pegamos o tempo da ultima modificacao
                src_files_modify[f] = os.path.getmtime(f)
                src_files_recompile[f] = True

        # Procura por arquivos modificados
        check_for_modified()

        # Agora tentamos a compilação
        if do_compile() != 0:
            print('Falha na compilação!')
        else:
            print('Projeto compilado com sucesso!')
            if do_link() != 0:
                print('Falha ao ligar o projeto!')
            else:
                print('Projeto construido com sucesso!')
        # salva as informações sobre essa compilação
        save_database()
    elif sys.argv[1] == 'clean':
        print('Limpando o projeto...')
        for f in src_files:
            if os.path.exists(src_files[f]):
                os.remove(src_files[f])
            if os.path.exists(project_img):
                os.remove(project_img)
            if os.path.exists('kernel.map'):
                os.remove('kernel.map')
            if os.path.exists(build_db):
                os.remove(build_db)
        os.system("rm obj/*")
    elif sys.argv[1] == 'run':
        print('Executando o projeto...')
        if os.path.exists(project_img):
            os.system('qemu-system-arm -sd sd.img -kernel %s -cpu arm1176 -m 256 -M raspi' % (project_img))
        else:
            print('Sistema não construido!')
    elif sys.argv[1] == 'debug':
        print('Debuggando o projeto...')
        if os.path.exists(project_img):
            os.system('qemu-system-arm -sd sd.img -s -S -kernel %s -cpu arm1176 -m 256 -M raspi'  % 'kernel.elf')#(project_img))
        else:
            print('Sistema não construido!')
    elif sys.argv[1] == 'copy':
        print('Copiando o projeto...')
        if os.path.exists('/media/marcos/RPIBOOT'):
            ret = os.system('cp %s /media/marcos/RPIBOOT/kernel.img' % (project_img))
            if ret != 0:
                print('Não foi possível copiar o kernel para o cartão!')
            else:
                print('Kernel copiado com sucesso!')
        else:
            print('Unidade de disco não existe!')
