import { useState } from "react";

type Props = {
  problemIds: string[];
};

const SubmissionForm = ({ problemIds }: Props) => {
  const [solverName, setSolverName] = useState<string>("manarimo-solver");
  const [problemId, setProblemId] = useState<string>("1");
  const [file, setFile] = useState<File>();
  return (
    <form
      onSubmit={async (e) => {
        e.preventDefault();
        if (file) {
          const content = await readFile(file);
          await fetch(
            `https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/solutions/submit?solver=${solverName}&problem_id=${problemId}`,
            {
              method: "POST",
              body: content,
            }
          );
        }
      }}
    >
      <input
        type="input"
        value={solverName}
        onChange={(e) => setSolverName(e.target.value)}
      />
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
      <button type="submit" disabled={!file || !solverName}>
        Submit
      </button>
    </form>
  );
};

export default SubmissionForm;

const readFile = (file: File) => {
  return new Promise<string>((resolve, reject) => {
    const reader = new FileReader();

    reader.addEventListener("load", () => {
      const { result } = reader;
      if (typeof result !== "string")
        throw TypeError("Reader did not return string.");
      resolve(result);
    });

    reader.addEventListener("error", () => {
      reject(reader.error);
    });

    reader.readAsText(file);
  });
};
