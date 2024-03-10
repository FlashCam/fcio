#ifdef __linux__
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L
#else
#undef _POSIX_C_SOURCE
#endif

#include <stdio.h>
#include <bufio.h>
#include <tmio.h>
#include <fcio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void print_buffer_ptrs(bufio_stream* in_stream, bufio_stream* out_stream)
{
  static size_t previous_in_head = 0;
  static size_t previous_in_tail = 0;
  static size_t previous_out_tail = 0;
  fprintf(stderr, "in: head = %p (%ld->%ld = %ld) tail = %p (%ld->%ld = %ld) fill_level = %ld\n",
    in_stream->input_buffer_base + in_stream->input_buffer_head,
    previous_in_head, in_stream->input_buffer_head, in_stream->input_buffer_head - previous_in_head,
    in_stream->input_buffer_base + in_stream->input_buffer_tail,
    previous_in_tail, in_stream->input_buffer_tail, in_stream->input_buffer_tail - previous_in_tail,
    in_stream->input_buffer_fill
  );
  fprintf(stderr, "out: tail = %p (%ld->%ld = %ld) size = %ld\n",
    out_stream->output_buffer_base + out_stream->output_buffer_tail,
    previous_out_tail, out_stream->output_buffer_tail, out_stream->output_buffer_tail - previous_out_tail, 
    out_stream->output_buffer_size
  );
  previous_in_head = in_stream->input_buffer_head;
  previous_in_tail = in_stream->input_buffer_tail;
  previous_out_tail = out_stream->output_buffer_tail;
}

void print_write_buffer_info(bufio_stream* out_stream, char* info)
{
  fprintf(stderr, "%s buffer_position %ld size %ld kB\n",
    info,
    out_stream->output_buffer_tail,
    out_stream->output_buffer_size/1024
  );

}

void print_record_from_buffer(char* buffer, int streamlen)
{
  fprintf(stderr, "... buffer %p streamlen %d bytes\n", buffer, streamlen);

  int i = 0;
  while (i < streamlen) {
    int frame_header = *(int*)(buffer+i);
    if (frame_header < 0) {
      if (frame_header == TMIO_PROTOCOL_TAG) {
        fprintf(stderr, "... protocol %s\n", buffer+i+sizeof(frame_header));
        i += sizeof(frame_header) + TMIO_PROTOCOL_SIZE;
      } else {
        fprintf(stderr, "...  tag %d\n", -frame_header);
        i += sizeof(frame_header);
      }
    }  else {
      fprintf(stderr, "...  msg %d bytes\n", frame_header);
      i += frame_header + sizeof(frame_header);

    }
  }
}

static inline void fprintf_filepos(int fd, const char* s)
{
  fprintf(stderr, "current-file-pos: %ld %s\n", lseek(fd, 0, SEEK_CUR), s);
}

static inline int open_shm()
{
  mode_t file_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
  int mode = O_RDWR | O_CREAT;
  const char* name = "/testfile.fcio";
  return shm_open(name, mode, file_flags);
}

static inline void test_multiple_fd()
{
  int fd0 = open_shm();
  int fd1 = open_shm();
  shm_unlink("/testfile.fcio");

  #define size 10
  char data_write[size] = {0};
  char data_read[size] = {0};
  for (int i = 0; i < size; i++)
    data_write[i] = i;
  
  fprintf_filepos(fd0, "fd0 before write");
  fprintf_filepos(fd1, "fd1 before write");
  ssize_t written_bytes = write(fd0, data_write, size);
  fprintf_filepos(fd0, "fd0 after write");
  fprintf_filepos(fd1, "fd1 after write");
  ssize_t read_bytes = read(fd1, data_read, size);
  fprintf_filepos(fd0, "fd0 after read");
  fprintf_filepos(fd1, "fd1 after read");

  fprintf(stderr, "written %d: %zi read %d: %zi\n", fd0, written_bytes, fd1, read_bytes);
  for (int i = 0 ; i < size; i++) {
    if (data_read[i] != data_write[i])
      fprintf(stderr, "%d %c %c\n", i, data_read[i], data_write[i]);
  }
}

char* open_mmap(int fd, size_t mapsize)
{
  int prot = PROT_READ;
  int flags = MAP_SHARED;
  off_t offset = 0;
  size_t filesize = lseek(fd, 0, SEEK_END);
  fprintf(stderr, "shm mem filesize: %ld\n", filesize);
  char* shm_buffer = mmap(NULL, filesize, prot, flags, fd, offset);

  if (shm_buffer == MAP_FAILED) {
    fprintf(stderr, "mmap failed with %s\n", strerror(errno));
    exit(1);
  } else {
    fprintf(stderr, "mmap success %p\n", shm_buffer);
  }

  return shm_buffer;
}

size_t get_record_size(int fd)
{
  size_t filesize = lseek(fd, 0, SEEK_CUR);
  lseek(fd, 0, SEEK_SET);
  return filesize;
}

int main(int argc, char* argv[])
{
  // FCIODebug(10);
  
  // char* filename = "/dev/null";1
  char* filename = argv[1];
  size_t bufsize_kb = 0;

  FCIOData* io = FCIOOpen(filename, 0, bufsize_kb);

  fprintf(stderr, "%s: fd %d\n", filename, FCIOFileno(io->ptmio));

  FCIOStream out = FCIOConnect("shmem://open",'w', 0, bufsize_kb);
  int fd = FCIOFileno(out);

  size_t mapsize = 1024 * 1024 * 100;

  char* shm_buffer = open_mmap(fd, mapsize);

  print_record_from_buffer(shm_buffer, FCIOGetRecordStreamSize(out, 'w'));

  char peer[1025] = {0};
  snprintf(peer,1024, "shmem://fd/%d", fd);
  FCIOData* testio = FCIOOpen(peer, 0, bufsize_kb);

  int tag;
  int loop_cnt = 0;
  while ((tag = FCIOGetRecord(io)) && tag > 0) {
    
    int datalen = FCIOPutRecord(out, io, tag);

    // size_t recordsize = get_record_size(fd);
    // print_record_from_buffer(shm_buffer, recordsize);

    int recv_tag = 0;
    int inner_loop_cnt = 0;
    do {
      fprintf_filepos(fd, "before testreader getrecord");
      recv_tag = FCIOGetRecord(testio);
      fprintf_filepos(fd, "after testreader getrecord");
      fprintf(stderr, "testio-tag: %d loop_cnt %d inner %d\n", recv_tag, loop_cnt, inner_loop_cnt);
      inner_loop_cnt++;
    } while (recv_tag > 0);

    // if (loop_cnt % 2 == 0) {
    //   int recv_tag = 1;
    //   int inner_loop_cnt = 0;
    //   while (recv_tag > 0) {

    //     fprintf_filepos(fd, "before test reader getrecord");
    //     fprintf(stderr, "testio-begin\n");
    //     recv_tag = FCIOGetRecord(testio);
    //     fprintf(stderr, "testio-tag: %d loop_cnt %d inner %d\n", recv_tag, loop_cnt, inner_loop_cnt);
    //     fprintf_filepos(fd, "after testreader getrecord");
    //     fprintf(stderr, "testio-end\n");
    //     fprintf_filepos(fd, "endloop");
    //     inner_loop_cnt++;
    //   }
    // }
    // lseek(fd, 0, SEEK_SET);
    loop_cnt++;
  } 
  
  return 0;
}
