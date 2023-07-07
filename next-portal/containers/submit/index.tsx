import SubmissionForm from "@/components/submit";
import useProblemList from "../top/hooks/useProblemList";

const SubmissionFormContainer = () => {
  const problems = useProblemList();
  const problemIds = problems.data?.map((problem) => problem.id) ?? [];
  return <SubmissionForm problemIds={problemIds} />;
};

export default SubmissionFormContainer;
