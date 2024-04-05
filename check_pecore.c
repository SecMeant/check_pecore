#define _GNU_SOURCE

#include <stdio.h>

#include <cpuid.h>

#include <sched.h>

static unsigned int get_max_cpu(void)
{
	return 23;
}

static int migrate_to_cpu(unsigned int cpu)
{
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);

	return sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

static void for_each_cpu(void (*func)(void))
{
	const unsigned int max_cpu = get_max_cpu();

	for (unsigned int i = 0; i <= max_cpu; ++i) {
		if (migrate_to_cpu(i)) {
			fprintf(stderr, "Failed to migrate to CPU %u\n", i);
			continue;
		}

		func();
	}
}

static void sched_getcpu_wrapper(void)
{
	printf("CPU: %i\n", sched_getcpu());
}

/* CPUID.1A.EAX[31-24] */
#define CORE_TYPE_PCORE (0x40)
#define CORE_TYPE_ECORE (0x20)

static void print_core_type(void)
{
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	const int mycpu  = sched_getcpu();

	__get_cpuid(0x1a, &eax, &ebx, &ecx, &edx);
	const unsigned int coretype = (eax & 0xff000000) >> 24;

	const char * coretype_str = NULL;
	switch (coretype) {
		case CORE_TYPE_PCORE:
			coretype_str = "pcore";
			break;

		case CORE_TYPE_ECORE:
			coretype_str = "ecore";
			break;

		default:
			coretype_str = "unknown";
	}

	printf("CPU %i: %s\n", mycpu, coretype_str);
}

int main(void)
{
	const unsigned int max_cpuid = __get_cpuid_max(0, NULL);

	if (max_cpuid < 0x1a) {
		fprintf(stderr, "This CPU doesn't support CPUID.1A.EAX[31-24]\n");
		return 1;
	}

	for_each_cpu(print_core_type);
}