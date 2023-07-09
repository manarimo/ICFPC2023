import SubmissionForm from "@/components/submit";
import useProblems from "../home/hooks/useProblems";

const SubmissionFormContainer = () => {
  const problems = useProblems();
  const problemIds = problems.data?.problems.map((problem) =>
    problem.problemId.toString()
  );
  return <SubmissionForm problemIds={problemIds ?? []} />;
};

export default SubmissionFormContainer;
