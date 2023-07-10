ALTER TABLE solutions ADD COLUMN tag VARCHAR(64);
CREATE INDEX solutions_by_tag ON solutions (tag, score);