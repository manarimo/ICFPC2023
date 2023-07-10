import Home from "@/components/home";
import useBestSolutions from "./hooks";
import { useState } from "react";

function normalize(tag: string): string | undefined {
  if (tag.trim() == '') {
    return undefined;
  }
  return tag;
}

const HomeContainer = () => {
  const [tag, setTag] = useState('');
  const problems = useBestSolutions(normalize(tag));

  if (problems.error) {
    return <p>{JSON.stringify(problems.error)}</p>;
  }
  if (!problems.data) {
    return <p>Loading...</p>;
  }

  return (
    <div>
      Tag: <input type="text" value={tag} onChange={(e) => setTag(e.currentTarget.value)}></input>
      <Home problems={problems.data} />
    </div>
  );
};

export default HomeContainer;
