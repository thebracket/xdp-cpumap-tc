#ifndef __XDP_IPHASH_TO_CPU_USER_COMMON_H
#define __XDP_IPHASH_TO_CPU_USER_COMMON_H

/* Exit return codes */
#define	EXIT_OK			0
#define EXIT_FAIL		1
#define EXIT_FAIL_OPTION	2
#define EXIT_FAIL_XDP		3
#define EXIT_FAIL_MAP		20
#define EXIT_FAIL_MAP_KEY	21
#define EXIT_FAIL_MAP_FILE	22
#define EXIT_FAIL_MAP_FS	23
#define EXIT_FAIL_IP		30
#define EXIT_FAIL_CPU		31
#define EXIT_FAIL_BPF		40
#define EXIT_FAIL_BPF_ELF	41
#define EXIT_FAIL_BPF_RELOCATE	42
#define IP_HASH_MAX		50000

static int verbose = 1;

/* Export eBPF maps for IPv4 iphash as a files
 * Gotcha need to mount:
 *   mount -t bpf bpf /sys/fs/bpf/
 */

static const char *file_cpu_map = "/sys/fs/bpf/file_cpu_map";
static const char *file_ip_hash = "/sys/fs/bpf/file_ip_hash";
static const char *file_cpus_available = "/sys/fs/bpf/file_cpus_available";
static const char *file_cpus_count = "/sys/fs/bpf/file_cpus_count";
static const char *file_cpus_iterator = "/sys/fs/bpf/file_cpus_iterator";
static const char *file_cpu_direction = "/sys/fs/bpf/file_cpu_direction";



/* Iphash operations */
#define ACTION_ADD	(1 << 0)
#define ACTION_DEL	(1 << 1)

static int iphash_modify(int fd, char *ip_string, unsigned int action, unsigned int cpu_idx)
{
	//printf ("In iphash_modify %i\n",cpu_idx);
	//unsigned int nr_cpus = bpf_num_possible_cpus();
	//__u64 values[nr_cpus];
	__u32 key;
	int res;
	unsigned int nr_cpus = bpf_num_possible_cpus();
    	if (cpu_idx+1 > nr_cpus || cpu_idx+1 < 0)
		return EXIT_FAIL_CPU;

	/* Convert IP-string into 32-bit network byte-order value */
	res = inet_pton(AF_INET, ip_string, &key);
	if (res <= 0) {
		if (res == 0)
			fprintf(stderr,
				"ERR: IPv4 \"%s\" not in presentation format\n",
				ip_string);
		else
			perror("inet_pton");
		return EXIT_FAIL_IP;
	}
	printf ("key: %i\n",key);
	if (action == ACTION_ADD) {
		//res = bpf_map_update_elem(fd, &key, &cpu_idx, BPF_NOEXIST);
		res = bpf_map_update_elem(fd, &key, &cpu_idx, BPF_ANY);
	} else if (action == ACTION_DEL) {
		res = bpf_map_delete_elem(fd, &key);
	} else {
		fprintf(stderr, "ERR: %s() invalid action 0x%x\n",
			__func__, action);
		return EXIT_FAIL_OPTION;
	}

	if (res != 0) { /* 0 == success */
		fprintf(stderr,
			"%s() IP:%s key:0x%X errno(%d/%s)",
			__func__, ip_string, key, errno, strerror(errno));

		if (errno == 17) {
			fprintf(stderr, ": Already in Iphash\n");
			return EXIT_OK;
		}
		fprintf(stderr, "\n");
		return EXIT_FAIL_MAP_KEY;
	}
	if (verbose)
		fprintf(stderr,
			"%s() IP:%s key:0x%X\n", __func__, ip_string, key);
	return EXIT_OK;
}
#endif
