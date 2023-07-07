import { useState } from "react";

type Props = {
  problemIds: string[];
};

const SubmissionForm = ({ problemIds }: Props) => {
  const [problemId, setProblemId] = useState<string>("1");
  const [file, setFile] = useState<File>();
  return (
    <form
      onSubmit={async (e) => {
        e.preventDefault();
        const formData = new FormData();
        formData.append("problemId", problemId);
        if (file) {
          formData.append("solution", file);
        }
        const request = new Request("/api/file", {
          method: "POST",
          body: formData,
        });
        const response = await fetch(request);
      }}
    >
      <input
        type="file"
        name="solution"
        accept="application/json"
        onChange={(e) => {
          if (e.target.files) {
            const file = e.target.files[0];
            setFile(file);
          }
        }}
      />
      <select value={problemId} onChange={(e) => setProblemId(e.target.value)}>
        {problemIds.map((id) => (
          <option key={id} value={id}>
            {id}
          </option>
        ))}
      </select>
      <button type="submit" disabled={!file}>
        Submit
      </button>
    </form>
  );
};

export default SubmissionForm;
