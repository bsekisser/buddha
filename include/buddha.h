/* from https://github.com/Spritetm/unbuddha/unbuddha.c */
typedef struct code_idx_ent_t* code_idx_ent_p;
typedef struct __attribute__((packed)) code_idx_ent_t {
	uint16_t idx;
	uint16_t len;
	uint16_t unk1; //0, possibly part of len
	uint16_t load_at;
	uint16_t unk2; //0, possibly part of load_at
	uint16_t offset;
	uint16_t dcrc;
	uint16_t tcrc;
} code_idx_ent_t;
