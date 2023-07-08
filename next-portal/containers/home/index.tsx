import Home from "@/components/home";
import useBestSolutions from "./hooks";

const HomeContainer = () => {
  const problems = useBestSolutions();
  if (problems.error) {
    return <p>{JSON.stringify(problems.error)}</p>;
  }
  if (!problems.data) {
    return <p>Loading...</p>;
  }

  return <Home problems={problems.data} />;
};

export default HomeContainer;
