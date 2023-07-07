CREATE TABLE problems (
    problem_id INTEGER NOT NULL,
    room_width INTEGER NOT NULL,
    room_height INTEGER NOT NULL,
    num_musicians INTEGER NOT NULL,
    num_attendees INTEGER NOT NULL,

    PRIMARY KEY (problem_id)
);

CREATE TABLE solutions (
    id SERIAL,
    solver_name VARCHAR(256) NOT NULL,
    problem_id INTEGER NOT NULL,
    score BIGINT NOT NULL,
    solution_path VARCHAR(256) NOT NULL,

    UNIQUE (solver_name, problem_id)
);

CREATE INDEX solutions_by_solver ON solutions (solver_name);
CREATE INDEX solutions_by_problem_id ON solutions (problem_id, score);
CREATE INDEX solutions_by_score ON solutions (score);